# 程式碼說明

處理影像，將其裁剪、放置於背景上，並生成不同旋轉角度的圖像。最終這些處理後的影像會被保存到指定的資料夾中。
## 主要流程

1. **影像處理：**
   - 程式會讀取指定資料夾中的影像，從中裁剪出中心部分，並將其置於指定大小的白色背景上。
   - 接著，程式會根據設定的旋轉角度增量生成旋轉後的影像，並將每個旋轉角度對應的影像保存。

2. **影像儲存：**
   - 每個旋轉後的影像會根據原始檔案名稱與旋轉角度來命名並儲存。

## 主要結構與功能

### 1. `processImage` 函數
該函數負責：
- 讀取指定的影像檔案。
- 裁剪影像的中心部分，並將其放置於大小為`background_size`的白色背景上。
- 根據`rotation_angle_increment`設定的角度增量，對影像進行旋轉並儲存每個旋轉角度對應的影像。

**步驟：**
- 使用OpenCV的`imread`讀取影像。
- 根據`crop_size`設定，裁剪影像的中心部分。
- 創建一個白色背景，並將裁剪後的影像置中於背景上。
- 依據`rotation_angle_increment`設置的角度進行旋轉，並保存每個旋轉後的影像。

### 2. `processDirectory` 函數
該函數負責遍歷指定資料夾中的所有影像檔案（`.png`或`.jpg`格式），並對每個影像執行`processImage`函數。

### 3. `main` 函數
程式的入口點，負責處理命令列參數，並調用`processDirectory`來處理指定資料夾中的影像。

**命令列參數：**
- `<input_folder>`: 包含原始影像的資料夾路徑。
- `<output_folder>`: 處理後的影像將被保存的資料夾路徑。
- `<background_size>`: 背景影像的大小。
- `<crop_size>`: 裁剪影像的大小。
- `<rotation_angle_increment>`: 每次旋轉的角度增量。

### 4. 影像旋轉
程式會使用OpenCV的`getRotationMatrix2D`來計算旋轉矩陣，並使用`warpAffine`函數對背景影像進行旋轉，生成不同角度的影像。

### 5. 輸出
- 旋轉後的影像會儲存在`output_folder`中，並按照原始檔名和旋轉角度命名，例如：`image_R30.jpg`表示旋轉30度後的影像。

## 輸入與輸出

### 輸入：
- `input_folder`: 原始影像所在的資料夾。
- `output_folder`: 處理後的影像儲存的資料夾。
- `background_size`: 背景影像的大小。
- `crop_size`: 裁剪影像的大小。
- `rotation_angle_increment`: 旋轉角度的增量。

### 輸出：
- 生成的旋轉後的影像會儲存在`output_folder`中，根據原始檔案名稱和旋轉角度命名。

## 依賴庫
- `opencv2`: 用於影像處理（讀取影像、裁剪、旋轉、儲存等）。
- `filesystem`: 用於操作檔案系統，遍歷資料夾、創建資料夾等。

## 結論

這段程式碼提供了一個簡單的影像處理工具，能夠對影像進行裁剪並將其放置在白色背景上，然後生成多個旋轉角度的影像，並保存到指定資料夾中。適用於需要生成多個不同角度的影像集的情況。