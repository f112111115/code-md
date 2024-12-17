//自動分配好YOLO需要的三個資料夾
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <cmath>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <random>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;
using json = nlohmann::json;

using MyPoint = pair<double, double>;

struct YoloAnnotation {
    int class_id;
    float x_center;
    float y_center;
    float width;
    float height;
};

unordered_map<string, int> label_map;
int class_id_counter = 0;

vector<YoloAnnotation> convertToYoloFormat(const json& shapes, int image_width, int image_height) {
    vector<YoloAnnotation> annotations;

    for (const auto& shape : shapes) {
        if (!shape.contains("label") || !shape.contains("points")) {
            cerr << "Error: JSON shape does not contain 'label' or 'points'." << endl;
            continue;
        }

        string label = shape["label"];

        if (label_map.find(label) == label_map.end()) {
            label_map[label] = class_id_counter++;
        }

        int class_id = label_map[label];
        const auto& points = shape["points"];

        float min_x = numeric_limits<float>::max();
        float max_x = numeric_limits<float>::lowest();
        float min_y = numeric_limits<float>::max();
        float max_y = numeric_limits<float>::lowest();

        for (const auto& point : points) {
            float x = point[0];
            float y = point[1];

            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }

        float x_center = ((min_x + max_x) / 2.0) / image_width;
        float y_center = ((min_y + max_y) / 2.0) / image_height;
        float width = (max_x - min_x) / image_width;
        float height = (max_y - min_y) / image_height;

        annotations.push_back({ class_id, x_center, y_center, width, height });
    }

    return annotations;
}

void saveYoloAnnotations(const vector<YoloAnnotation>& annotations, const string& output_path) {
    ofstream output_file(output_path);
    if (!output_file.is_open()) {
        cerr << "Error: Unable to create output file: " << output_path << endl;
        return;
    }

    for (const auto& annotation : annotations) {
        output_file << annotation.class_id << " "
            << annotation.x_center << " "
            << annotation.y_center << " "
            << annotation.width << " "
            << annotation.height << "\n";
    }

    output_file.close();
}

void randomSplitAndCopy(const string& output_folder, const string& yolo_folder) {
    vector<fs::path> all_images, all_labels;

    // Collect all images and labels
    for (const auto& entry : fs::recursive_directory_iterator(output_folder)) {
        if (entry.is_regular_file()) {
            if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png") {
                all_images.push_back(entry.path());
            }
            else if (entry.path().extension() == ".txt") {
                all_labels.push_back(entry.path());
            }
        }
    }

    // Calculate split sizes
    size_t total = all_images.size();
    size_t train_size = static_cast<size_t>(0.7 * total);
    size_t val_size = static_cast<size_t>(0.225 * total);
    size_t test_size = total - train_size - val_size;

    // Shuffle indices
    vector<size_t> indices(total);
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), std::mt19937{ std::random_device{}() });

    // Function to copy files
    auto copy_files = [&](const vector<size_t>& indices, const string& type) {
        string img_folder = yolo_folder + "/" + type + "/images/";
        string lbl_folder = yolo_folder + "/" + type + "/labels/";

        fs::create_directories(img_folder);
        fs::create_directories(lbl_folder);

        for (size_t idx : indices) {
            fs::copy(all_images[idx], img_folder + all_images[idx].filename().string());
            fs::copy(all_labels[idx], lbl_folder + all_labels[idx].filename().string());
        }
        };

    // Copy train, val, test datasets
    copy_files(vector<size_t>(indices.begin(), indices.begin() + train_size), "train");
    copy_files(vector<size_t>(indices.begin() + train_size, indices.begin() + train_size + val_size), "val");
    copy_files(vector<size_t>(indices.begin() + train_size + val_size, indices.end()), "test");
}

void processImageAndJson(const string& input_path, const string& input_folder, const string& output_folder, int background_size, int crop_size, double rotation_angle_increment, const string& json_path) {
    Mat img = imread(input_path);
    if (img.empty()) {
        cerr << "Error: Unable to read input image: " << input_path << endl;
        return;
    }

    int start_row = (img.rows - crop_size) / 2;
    int start_col = (img.cols - crop_size) / 2;
    Mat cropped_img = img(Rect(start_col, start_row, crop_size, crop_size));

    Mat background(background_size, background_size, img.type(), Scalar(255, 255, 255));

    int offset_x = (background.cols - crop_size) / 2;
    int offset_y = (background.rows - crop_size) / 2;

    cropped_img.copyTo(background(Rect(offset_x, offset_y, crop_size, crop_size)));

    fs::path input_path_fs(input_path);
    string relative_path = fs::relative(input_path_fs.parent_path(), fs::path(input_folder)).string();
    string output_subfolder = output_folder + "/" + relative_path;
    fs::create_directories(output_subfolder);

    json jsonData;
    if (!json_path.empty()) {
        ifstream file(json_path);
        if (file.is_open()) {
            file >> jsonData;
            file.close();
        }
        else {
            cerr << "Error: Unable to read input JSON: " << json_path << endl;
            return;
        }
    }

    string base_filename = input_path_fs.stem().string();
    size_t pos = base_filename.find("_R");
    if (pos != string::npos) {
        base_filename = base_filename.substr(0, pos);
    }

    for (double angle = 0; angle < 360; angle += rotation_angle_increment) {
        Mat rotation_matrix = getRotationMatrix2D(Point2f(background.cols / 2.0, background.rows / 2.0), -angle, 1);
        Mat rotated_background;
        warpAffine(background, rotated_background, rotation_matrix, background.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(255, 255, 255));

        string angle_str = "_R" + to_string(static_cast<int>(angle));
        string output_filename = base_filename + angle_str + ".jpg";
        string output_path = output_subfolder + "/" + output_filename;
        imwrite(output_path, rotated_background);

        if (!jsonData.is_null()) {
            json rotatedJsonData = jsonData;

            for (auto& shape : rotatedJsonData["shapes"]) {
                for (auto& point : shape["points"]) {
                    Mat pt = (Mat_<double>(3, 1) << point[0], point[1], 1);
                    Mat rotated_pt = rotation_matrix * pt;
                    point[0] = rotated_pt.at<double>(0, 0);
                    point[1] = rotated_pt.at<double>(1, 0);
                }
            }

            vector<YoloAnnotation> yolo_annotations = convertToYoloFormat(rotatedJsonData["shapes"], background_size, background_size);

            string output_txt_filename = base_filename + angle_str + ".txt";
            string output_txt_path = output_subfolder + "/" + output_txt_filename;
            saveYoloAnnotations(yolo_annotations, output_txt_path);
        }
    }
}

void processDirectory(const string& input_folder, const string& output_folder, const string& yolo_folder, int background_size, int crop_size, double rotation_angle_increment) {
    for (const auto& entry : fs::recursive_directory_iterator(input_folder)) {
        if (entry.is_regular_file()) {
            string input_path = entry.path().string();
            if (entry.path().extension() == ".png" || entry.path().extension() == ".jpg") {
                fs::path json_path_candidate = entry.path();
                string json_path = json_path_candidate.replace_extension(".json").string();
                processImageAndJson(input_path, input_folder, output_folder, background_size, crop_size, rotation_angle_increment, json_path);
            }
        }
    }

    randomSplitAndCopy(output_folder, yolo_folder);
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " <input_folder> <output_folder> <yolo_folder> <background_size> <crop_size> <rotation_angle_increment>" << endl;
        return 1;
    }

    string input_folder = argv[1];
    string output_folder = argv[2];
    string yolo_folder = argv[3];
    int background_size = stoi(argv[4]);
    int crop_size = stoi(argv[5]);
    double rotation_angle_increment = stod(argv[6]);

    processDirectory(input_folder, output_folder, yolo_folder, background_size, crop_size, rotation_angle_increment);

    return 0;
}
