# Copyright (C) 2024 wwhai
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import cv2

# 读取 YUV 文件
yuv_file = "./yuv/1.yuv"
width, height = 1920, 1080  # YUV 文件的分辨率
yuv_data = open(yuv_file, "rb").read()

# 解析 YUV 数据并转换为图像
img_yuv = cv2.cvtColor(yuv_data, cv2.COLOR_YUV2BGR_I420)  # 假设是 YUV420 格式
img_rgb = cv2.cvtColor(img_yuv, cv2.COLOR_YUV2RGB)

# 显示图像
cv2.imshow("YUV Image", img_rgb)
cv2.waitKey(0)
cv2.destroyAllWindows()
