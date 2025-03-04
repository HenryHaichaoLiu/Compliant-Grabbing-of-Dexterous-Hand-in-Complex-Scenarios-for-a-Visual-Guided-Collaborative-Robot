# Copyright 2016 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import random

# import torch
import torch.utils.data
# import librealsense2
import cv2
import numpy as np
from skimage.filters import gaussian
from models.common import post_process_output
from utils.dataset_processing import evaluation, grasp
import math
from align_depth_color import *


# class GGCNNReady:
#
#     def __init__(self):
#         self.AlignOK = False
#         self.ColorOK = False
#         self.GgcnnOK = False
#
#     def set_align_ok(self, setflag):
#         self.AlignOK = setflag
#         if self.ColorOK:
#             self.GgcnnOK = True
#
#     def set_color_ok(self, setflag):
#         self.ColorOK = setflag
#         if self.AlignOK:
#             self.GgcnnOK = True
#
#     def get_ggcnn_ok(self):
#         return self.GgcnnOK
#
#     def refresh_ok(self):
#         self.AlignOK = False
#         self.ColorOK = False
#         self.GgcnnOK = False
#
#
# picture_ready = GGCNNReady()

from scipy import ndimage as ndi
import matplotlib.pyplot as plt
from skimage.feature import peak_local_max
from skimage import data, img_as_float


class GraspGenerater:

    def __init__(self):

        # 网络初始化
        self.net = torch.load("./ggcnn_weights_cornell/epoch_45_iou_0.76")
        self.device = torch.device("cuda:0")
        self.color_image = np.zeros([480, 640, 3], np.uint8)
        self.depth_image = np.zeros([480, 640])
        self.color_after_crop = np.zeros([300, 300, 3], np.uint8)

        self.ggcnn_input = np.zeros([300, 300])
        self.bias = np.zeros(2)
        self.font_depth = 0
        self.back_depth = 0
        self.color_x = 0
        self.color_y = 0

        self.output_data = []

    # output_size = 300,shape = [640, 480]
    # 以下函数用于图像裁剪
    def get_crop_center(self):
        # dst = self.delete_zero(self.depth_image)
        # dst = self.depth_image / 64
        dst = self.delete_zero(self.depth_image)
        # 根据颜色进行前景提取
        cv2.imshow("color_image", self.color_image)
        gray = cv2.cvtColor(self.color_image, cv2.COLOR_BGR2GRAY)
        mask = (gray < 75).astype(np.float)
        gray = (gray * mask).astype(np.uint8)

        # # mask = (dst < 8.8).astype(np.float)
        # #计算背景平均距离
        self.back_depth = (dst * (1 - mask)).sum() / (640 * 480 - mask.sum()) / 1000
        # #计算前景平均距离
        self.font_depth = (dst * mask).sum() / mask.sum() / 1000
        # dst = dst * mask
        mask = ((dst != 0) & (dst <= (self.font_depth) * 1000)).astype(np.float)
        # print("mask_sum:", mask.sum())
        self.font_depth = (dst * mask).sum() / mask.sum() / 1000

        mask = ((dst != 0) & (dst <= (self.font_depth) * 1000)).astype(np.float)
        print("mask_sum:", mask.sum())
        self.font_depth = (dst * mask).sum() / mask.sum() / 1000

        # dst = cv2.cvtColor(dst, cv2.GR)#将图像转化为灰度图像

        # dst = cv2.Canny(dst * 4, 100, 150)#Canny边缘检测
        # dst = (dst * mask).astype(np.uint8)
        cv2.imshow("depth_canny", gray)
        # print("front_depth:",self.front_depth)
        # calculate moments of binary image
        M = cv2.moments(gray)
        # calculate x,y coordinate of center
        cX = int(M["m10"] / M["m00"])
        cY = int(M["m01"] / M["m00"])
        self.color_x = cX
        self.color_y = cY
        center = [cY, cX]

        return center

    def get_crop_attr(self, center, output_size, shape):

        # 自主检测目标大致位置模式
        if center == [-1, -1]:
            center = self.get_crop_center()

        # 人为设定裁剪位置
        else:
            pass

        left = max(0, min(center[1] - output_size // 2, shape[0] - output_size))
        top = max(0, min(center[0] - output_size // 2, shape[1] - output_size))
        self.bias[0] = left
        self.bias[1] = top

        return center, left, top

    def delete_zero(self, img):
        dst = cv2.inpaint(img, (img == 0).astype(np.uint8), 3, cv2.INPAINT_NS)
        return dst

    def picture_crop(self, img, left, top, type):
        output_size = 300
        img_output = np.zeros((300, 300))
        if type == "rgb":
            img_output = img[top:min(480, top + output_size), left:min(640, left + output_size)]
            img_output.reshape((output_size, output_size, 3))
        elif type == "depth":
            img_output = img[top:min(480, top + output_size), left:min(640, left + output_size)]
            img_output.reshape((output_size, output_size))
        elif type == "depth_norm":
            img = cv2.medianBlur(img, 5)
            # img = cv2.medianBlur(img, 5)
            # img = cv2.morphologyEx(
            #     img, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_RECT, (7, 7)))

            img_output = img[top:min(480, top + output_size), left:min(640, left + output_size)].astype(
                np.float32) / 1000
            img_output = self.delete_zero(img_output)
            img_output = np.clip((img_output - img_output.mean()), -1, 1)
            img_output.reshape((output_size, output_size))

        return img_output

    # 获取裁剪后的彩色、深度图
    def get_ggcnn_input(self, center):
        # start_col = int((center[0]-center[1])/2)
        # end_col = int(center[0]-start_col)
        # cropped_color = self.color_image[:, start_col:end_col]
        # cropped_depth = self.depth_image[:, start_col:end_col]
        # self.color_after_crop = cv2.resize(cropped_color, (300, 300))
        # self.depth_after_crop = cv2.resize(cropped_depth, (300, 300))
        # # self.ggcnn_input = np.concatenate((self.color_after_crop, np.expand_dims(self.depth_after_crop, axis=2)), axis=2)
        # # self.picture_crop(self.depth_image, left, top, "depth_norm")
        # self.ggcnn_input = self.depth_after_crop
        _, left, top = self.get_crop_attr(center, 300, [640, 480])
        self.color_after_crop = self.picture_crop(self.color_image, left, top, "rgb")
        self.ggcnn_input = self.picture_crop(self.depth_image, left, top, "depth_norm")

    def numpy_to_torch(self, s):
        if len(s.shape) == 2:
            return torch.from_numpy(np.expand_dims(s, 0).astype(np.float32))
        else:
            return torch.from_numpy(s.astype(np.float32))

    # 输出抓取点参数
    def get_ggcnn_output(self, vis=True):
        with torch.no_grad():
            x = self.numpy_to_torch(self.ggcnn_input).to(self.device)
            x = torch.unsqueeze(x, dim=0)
            pos_output, cos_output, sin_output, width_output = self.net(x)
            q_img, ang_img, width_img = post_process_output(pos_output, cos_output,
                                                            sin_output, width_output)
            # if vis:
            x, y, angle, width = evaluation.plot_output(self.color_after_crop,
                                                        self.ggcnn_input, q_img,
                                                        ang_img, 1, grasp_width_img=width_img)
            # if x!=-1 and y!=-1:
            #     self.last_gs[2] = 1
            #     self.last_gs[0] = x
            #     self.last_gs[1] = y

            cv2.circle(self.color_after_crop, (x, y), 5, [255, 0, 0], 4)
            # m = math.sqrt(5) * width
            # self.color_after_crop = cv2.rectangle(self.color_after_crop, (math.ceil(x+0.5*m*math.cos(angle)), math.ceil(y-0.5*m*math.sin(angle))),
            #                                     (math.ceil(x-0.5*m*math.cos(angle)), math.ceil(y+0.5*m*math.sin(angle))), (0, 255, 0), 2)
            alpha = math.pi / 2 - angle
            p = []
            p.append((math.ceil(x + width * math.cos(angle) - 0.5 * width * math.cos(alpha)),
                      math.ceil(y - width * math.sin(angle) - 0.5 * width * math.sin(alpha))))
            p.append((math.ceil(x + width * math.cos(angle) + 0.5 * width * math.cos(alpha)),
                      math.ceil(y - width * math.sin(angle) + 0.5 * width * math.sin(alpha))))
            p.append((math.ceil(x - width * math.cos(angle) + 0.5 * width * math.cos(alpha)),
                      math.ceil(y + width * math.sin(angle) + 0.5 * width * math.sin(alpha))))
            p.append((math.ceil(x - width * math.cos(angle) - 0.5 * width * math.cos(alpha)),
                      math.ceil(y + width * math.sin(angle) - 0.5 * width * math.sin(alpha))))
            for i in range(0, 3):
                cv2.line(self.color_after_crop, p[i], p[i + 1], [0, 255, 0], 2)
            cv2.line(self.color_after_crop, p[3], p[0], [0, 255, 0], 2)


            output_data = [self.color_x, self.color_y, angle, width, self.font_depth, self.back_depth]

            print("[x:" + str(x + self.bias[0]) + "," + "y:" + str(y + self.bias[1]) + "," + "theta:" + str(angle) + "," + "width:" + str(width) + "," +  "front:" + str(self.font_depth) + "," + "back:" + str(self.back_depth) + "]")
            cv2.imshow("grasp", self.color_after_crop)
            cv2.imshow("depth", (self.ggcnn_input * 128 + 128).astype(np.uint8))
            cv2.waitKey(1)


    def getImgInput(self, color_img, depth_img):
        self.color_image = color_img
        self.depth_image = depth_img

    def getOutputData(self):
        return self.output_data
def main(args=None):
    GGCNNTool = GraspGenerater()
    colorimg_name = "color"
    # alignimg_name = "align"
    init_RS_images()
    i=1
    while i:
        # 获取RGB、 DEPTH图片
        color_img, depth_img = send_RS_images()
        # cv2.imshow("color", color_img)
        # cv2.imshow("depth", depth_img*16)

        GGCNNTool.getImgInput(color_img, depth_img)
        GGCNNTool.get_ggcnn_input([-1, -1])
        GGCNNTool.get_ggcnn_output()
        cv2.waitKey(1)
        # GGCNNTool.GGCNNOutputPublish()
        # GGCNNTool.color_image = cv2.cvtColor(GGCNNTool.color_image, cv2.COLOR_BGR2RGB)
        # cv2.imshow(colorimg_name, GGCNNTool.color_image)
        # cv2.waitKey(1)
        # rclpy.spin(minimal_publisher)
        # a = torch.rand(3, 3)
        # print(a)
        # Destroy the node explicitly
        # (optional - otherwise it will be done automatically
        # when the garbage collector destroys the node object)
        # i -= 1


if __name__ == '__main__':
    main()
