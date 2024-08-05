#include "opencv2/opencv.hpp"
#include "iostream"
#include "bitset"
#include "vector"
#include "sstream"
#include "chrono"
#include "thread"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

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
            if (num_chars == count_bits / 8) break;
            Vec3b pixel = img.at<Vec3b>(i,j);
            ss << retrieve_bits_from_pixel(pixel, count_bits, num_chars);
            if (count_bits >= 8 && count_found == false) {
                count_found = true;
                string bits(8, '\0');
                ss.read(&bits[0], 8);
                num_chars = bitset<8>(bits).to_ulong();
                num_chars++;
            }
        }
    }
    string binary = ss.str();
    string revealed_text = "";
    for (size_t i = 1; i < binary.length() / 8; i++) {
        bitset<8> bits = bitset<8>(binary.substr(i * 8, 8));
        revealed_text = revealed_text + static_cast<char>(bits.to_ulong());
    }
    return revealed_text;
}

string conv_msg_binary(string msg) {
    string binary_number_letters = bitset<8>(msg.length()).to_string();
    stringstream ss;
    for (char c : msg) {
        ss << (bitset<8>(c).to_string());
    }
    return binary_number_letters + ss.str();
}

int main(int argc, char *argv[])
{
    int flag = -1, action = -1;
    string msg, path, destination;
    while ((flag = getopt(argc, argv, "edm:p:hs:")) != -1) {
        switch (flag) {
            case 'e': {
                if (action != -1) {puts("Error: Cannot both embed a message and retrieve a message in one command"); exit(-1);}
                action = 1;
                break;
            }
            case 'd': {
                if (action != -1) {puts("Error: Cannot both embed a message and retrieve a message in one command"); exit(-1);}
                action = 2;
                break;
            }
            case 'm': {
                msg = string(optarg);
                break;
            }
            case 's': {
                destination = string(optarg);
                break;
            }
            case 'p': {
                path = string(optarg);
                break;
            }
            case 'h': {
                puts("This steganography tool can embed a message into an image, or retrieve an embedded message from an image. Created images that contain messages will appear undistinguishable from normal images to the human eye.\n");
                puts("Command line flags:");
                puts("-e This flag specifies that you want to embed a message into an image. No argument. WARNING! Due to embedding method, the resulting embedded image must be of a lossless file format. png is recommended.");
                puts("-d This flag specifies that you want to retrieve a message from an image. No argument.");
                puts("-m This flag specifies the message that you want to embed in the image. Message must be between 1-255 characters. Argument required.");
                puts("-p This flag specifies the path of the image you either want to create, or retrieve a message from. Argument required.");
                puts("-s This flag specifies the path of the newly created image with the embedded message. Argument required");
                puts("-h This flag prints out information regarding the use of the program. No argument");
                return 0;
                break;
            }
            case '?': {
                cout << "Error: Invalid flag: " << optarg << endl;
                break;
            }
        }
    }
    switch (action) {
        case 1: {
            if (msg.empty() || msg.length() == 0 || msg.length() > 255) {
                puts("Error: Message must be entered and be within 1 to 255 characters");
                exit(-1);
            }
            if (path.empty()) {
                puts("Error: Path must be specified");
                exit(-1);
            }
            if (destination.empty()) {
                puts("Error: Destination path must be specified");
                exit(-1);
            }
            Mat img = imread(path, IMREAD_COLOR);
            cout << "Embedding image within " << destination << "..." << endl;
            embed_message(img, conv_msg_binary(msg));
            imwrite(destination, img);
            cout << "Done. Embedded image can be found at " << destination << endl; 
            break;
        }
        case 2: {
            if (path.empty()) {
                puts("Error: Path must be specified");
                exit(-1);
            }
            Mat img = imread(path, IMREAD_COLOR);
            cout << "Retrieving message from " << path << "..." << endl;
            string retrieved_msg = retrieve_message(img);
            puts("Done.");
            cout << "Retrieved message: " << retrieved_msg << endl;
            break;
        }
        case -1: {
            puts("Error: Must specify if you want to embed or retrieve a message");
            exit(-1);
        }
    }
    return 0;
}