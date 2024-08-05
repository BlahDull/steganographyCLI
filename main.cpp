#include "opencv2/opencv.hpp"
#include "iostream"
#include "bitset"
#include "vector"
#include "sstream"
#include "chrono"
#include "thread"

using namespace cv;
using namespace std;

void view_colors(Mat img) {
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            Vec3b color = img.at<Vec3b>(i, j);
            cout << bitset<8>(color[0]) << " " << bitset<8>(color[1]) << " " << bitset<8>(color[2]) << endl; 
            std::this_thread::sleep_for(chrono::seconds(1));
        }
    }
}

Vec3b set_bits(Vec3b color, size_t &current_bit, size_t max_bits, string msg_bits) {
    bitset<8> red = bitset<8>(color[0]);
    bitset<8> green = bitset<8>(color[1]);
    bitset<8> blue = bitset<8>(color[2]);
    red = red.set(0, msg_bits.at(current_bit) - '0');
    current_bit = current_bit + 1;
    if (current_bit > max_bits) return Vec3b(red.to_ulong(), green.to_ulong(), blue.to_ulong());
    green = green.set(0, msg_bits.at(current_bit) - '0');
    current_bit = current_bit + 1;
    if (current_bit > max_bits) return Vec3b(red.to_ulong(), green.to_ulong(), blue.to_ulong());
    blue = blue.set(0, msg_bits.at(current_bit) - '0');
    current_bit = current_bit + 1;
    Vec3b new_rgb(red.to_ulong(), green.to_ulong(), blue.to_ulong());
    return new_rgb;
}


void embed_message(Mat img, string msg_bits) {
    size_t current_bit = 0;
    size_t num_bits = msg_bits.length() - 1;
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            if (current_bit > num_bits) break;
            Vec3b pixel = img.at<Vec3b>(i, j);
            auto rgb = set_bits(pixel, current_bit, num_bits, msg_bits);
            img.at<Vec3b>(i, j) = rgb;
            if (current_bit > num_bits) break;
        }
    } 
}

string retrieve_bits_from_pixel(Vec3b color, int &count_bits, int num_char) {
    stringstream ss;
    bitset<8> red = bitset<8>(color[0]);
    count_bits++;
    if (count_bits / 8 == num_char) {
        ss << to_string(red[0]);
        return ss.str();
    }
    bitset<8> green = bitset<8>(color[1]);
    count_bits++;
    if (count_bits / 8 == num_char) {
        ss << to_string(red[0]) << to_string(green[0]);
        return ss.str();
    }
    bitset<8> blue = bitset<8>(color[2]);
    count_bits++;
    ss << to_string(red[0]) << to_string(green[0]) << to_string(blue[0]);
    return ss.str();
}

string retrieve_message(Mat img) {
    int num_chars = -1, count_bits = 0;
    bool count_found = false;
    stringstream ss;
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            cout << count_bits << endl;
            if (num_chars == count_bits / 8) break;
            Vec3b pixel = img.at<Vec3b>(i,j);
            ss << retrieve_bits_from_pixel(pixel, count_bits, num_chars);
            // cout << ss.str() << endl;
            if (count_bits >= 8 && count_found == false) {
                count_found = true;
                string bits(8, '\0');
                ss.read(&bits[0], 8);
                num_chars = bitset<8>(bits).to_ulong();
                num_chars++;
                cout << num_chars << endl;
                std::this_thread::sleep_for(chrono::seconds(1));
            }
        }
    }
    string binary = ss.str();
    string revealed_text = "";
    cout << binary << endl;
    for (size_t i = 1; i < binary.length() / 8; i++) {
        bitset<8> bits = bitset<8>(binary.substr(i * 8, 8));
        cout << static_cast<char>(bits.to_ulong()) << endl;
        revealed_text = revealed_text + static_cast<char>(bits.to_ulong());
    }
    return revealed_text;
}

string conv_msg_binary(string msg) {
    string binary_number_letters = bitset<8>(msg.length()).to_string();
    cout << msg << endl;
    stringstream ss;
    for (char c : msg) {
        ss << (bitset<8>(c).to_string());
    }
    return binary_number_letters + ss.str();
}
// message must be 255 char or less
int main()
{
    string msg = "You can fit a 4090 STRIX inside the Lian Li A3, but it's gonna be close. The SFX-L PSU has to be in position F2.5. Motherboard layout must have a top positioned PCIe x16 slot to fit three slim AF120 fans underneath this GPU.";
    auto msg_bits = conv_msg_binary(msg);
    cout << msg_bits << endl;
    string image_path = "test2.jpg";
    Mat img = imread(image_path, IMREAD_COLOR);
    embed_message(img, msg_bits);
    imwrite("embedded_images/image.png", img);
    image_path = "embedded_images/image.png";
    img = imread(image_path, IMREAD_COLOR);
    cout << retrieve_message(img);
    return 0;
}