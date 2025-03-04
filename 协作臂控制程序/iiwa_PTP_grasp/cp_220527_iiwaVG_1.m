clc;clear;
close all;
% 接收RealSenseD435i目标点像素与旋转角的kuka iiwa的视觉引导程序
% 使用了KST类
% 直接接收中心点坐标和旋转角度
% 收数据只收一次
%% experience
% Loc和tec控制机器人手的左右偏移和下降高度。
%% code
InspireHandOpen = "E:\VS2019_ws\InspireHandOpen\InspireHandOpen_lhc\x64\Release\InspireHandOpen_lhc.exe";
InspireHandClose = "E:\VS2019_ws\InspireHandClose\InspireHandClose_lhc\x64\Release\InspireHandClose_lhc.exe";
%% 连接机器人
warning('off');
ip='172.31.1.147';
arg1=daobanKST.LBR14R820; % choose the robot iiwa7R800 or iiwa14R820
arg2=daobanKST.Medien_Flansch_elektrisch; % choose the type of flange
Tef_flange=eye(4); % transofrm matrix of EEF with respect to flange
iiwa = daobanKST(ip,arg1,arg2,Tef_flange); % create the object
flag = iiwa.net_establishConnection();
if flag==0
    disp('Connection could not be establised, script aborted');
    return;
end
pause(1);
disp('Go!');
%% 参数初始化
% 预设机器人DH参数（已经写在kuka_jacobi里面）
[robot, L, jointsNum] = IBVS_createRobot('iiwa');
% 读取相机参数（预设相机参数）    % 需要一个直接从相机读参数的程序
camParam = [613 2.753e-06 2.753e-06 328 231 640 480];

% 机器人末端到相机前端（事件相机取本体）距离（单位：m）
Lcamera = 0.12;
% 希望预留的相机与物体距离
Loc = 0.10; % 相机额外加上灵巧手的长度
EPoffset = [0 0 Lcamera+Loc]';
% 机器人末端与相机之间的转换矩阵
TEC = [0 1 0 0.04;
    -1 0 0 -0.035+0.06;
    0 0 1 Lcamera;
    0 0 0 1];


%% 预设一些矩阵及标志位
CaptureNum = 20;
CandRz = zeros(3,CaptureNum); % 接收的中心点坐标与旋转角(deg)
% overtimeflag = 1; % 为1说明该周期超时（其实是说明超出视野）
safetyflag = 0; % 为0说明超出安全范围不能动
trycatchflag = 0; % 为1说明进入了catch异常处理当中

%% 初始化运动到固定位置并张开灵巧手，这个初始位置在白桌那一侧
% qinit = [0.0841, 0.7337, 0.0792, -1.1393, -0.0578, 1.2520, 0.1603]';
qinit_deg = [4.55, 36.69, 4.53, -41.59, -2.75, 100.81, 6.30]';
qinit = qinit_deg*pi/180;
jPosinit = mat2cell(qinit,[1 1 1 1 1 1 1])';
relVel = 0.2; % relative velocity
dos(InspireHandOpen);
[ret] = iiwa.movePTPJointSpace(jPosinit, relVel);
% pause(1);
disp('Go to initial position.');

% 读取机器人齐次变换矩阵
rMat = iiwa.getEEFOrientationR();
Pos = iiwa.getEEFCartesianPosition();
T_robotinit(1:3,1:3) = rMat;
T_robotinit(1:3,4) = cell2mat(Pos)/1000; % 注意将mm转到m
T_robotinit(4,4) = 1;
T_caminit = T_robotinit * TEC;

% 读取机器人末端位置姿态
[gep] = iiwa.getEEFPos();
p_eef = cell2mat(gep)';

% iiwa.realTime_startDirectServoCartesian();
% pause(1);
disp('Devices are ready.');
tic;

%% Visual Guidance Feedback
% 期望深度（由像素坐标反算相机坐标时的理论深度）
VG_output = python_test();
if(VG_output == "1")
    error("Object Grasp Detection Failed!");
end
time_1 = toc-tic;
disp(time_1);
VG_num = str2num(VG_output); % [center_x, center_y, alpha, open_width, obj_depth, platform_depth]
Zdepth = VG_num(5)/1000;
%% 接收目标物体中心点坐标与转正所需旋转角度
% % 多次接收
% for i = 1:1:CaptureNum
%     t0 = toc;
%     if(i>1)
%         [CandRz(:, i),overtimeflag] = iiwaVS_tcpgetcenter(serverIP,t0,CandRz(:, i-1));
%     else
%         [CandRz(:, i),overtimeflag] = iiwaVS_tcpgetcenter(serverIP,t0,CandRz(:, 1));
%     end
% end
% meanCandRz = mean(CandRz,2);
% 只收一次
t0 = toc;
%[meanCandRz,overtimeflag] = iiwaVS_tcpgetcenter(serverIP,t0,CandRz(:, 1));
meanCandRz = VG_num(1:3);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
disp('Have got image infomation.');
centerpixel = meanCandRz(1:2); EEF_Rz_plus = meanCandRz(3);
% centerpixel(1) = 640-centerpixel(1);
% centerpixel(2) = 640-centerpixel(2);
disp('result: '); disp(centerpixel); disp(meanCandRz(3));

% centerpixel = [93;93]; EEF_Rz_plus = 0.69;

%% 计算目标在基系中位置 计算末端应达位置
P_inCamera = iiwaVS_imgcenter2cam(camParam, centerpixel, Zdepth);
[ gjp ] = iiwa.getJointsPos();
qinit = cell2mat(gjp);
P_inBase = iiwaVS_cam2base(T_caminit, P_inCamera);
EEFgoto_inBase = P_inBase + EPoffset;
% 判断是否在可达空间（参数已调整）
[safetyflag, EEFgoto_inBase] = iiwaVS_isPosOK(EEFgoto_inBase);
% safetyflag = 1;

%% 计算相机转到目标方向所需角度
% 尽管分析计算的时候都在绕z轴方向旋转，但示教器手动操作结论是Rz与Tx同轴，即x和z轴的旋转角度控制互换了
p_eef(4:6) = iiwaVS_Rze2b(EEF_Rz_plus, rMat, p_eef(4:6));

%% 机器人末端运动，随后灵巧手抓取，随后抬升100mm并张开抓手
if (safetyflag)
    try
%         disp(p_eef(1:3));
        p_eef(1:3) = EEFgoto_inBase*1000; % 米转毫米
%         disp(p_eef(1:3));
        % 发送末端位置
        % 在sendEEfPosition的时候发送的是mm单位下的位置
        sep = mat2cell(p_eef,[1 1 1 1 1 1]);
%         [ret2] = iiwa.sendEEfPositions(sep);
        vel = 30;
        %dos(InspireHandOpen);
        [ret3] = iiwa.movePTPLineEEF(sep, vel);
        dos(InspireHandClose);
        p_eef(3)=p_eef(3)+100;
        sep = mat2cell(p_eef,[1 1 1 1 1 1]);
        [ret4] = iiwa.movePTPLineEEF(sep, vel);
        dos(InspireHandOpen);
        disp('VG OK');
        
    catch exception
        trycatchflag = 1;
        iiwa.net_turnOffServer();
        disp('Error during execution the direct servo motion');
        warning('on');
    end
else
    disp('Your target position is out of range.');
end


%% 断开机器人连接
% iiwa.realTime_stopDirectServoCartesian();
if(trycatchflag)
    disp('Closed');
else
    iiwa.net_turnOffServer();
    disp('Completed successfully');
    warning('on');
end

toc;
% 该版本没有多少输出的数据，故不画图



