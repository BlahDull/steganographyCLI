#include "opencv2/opencv.hpp"
#include "iostream"
#include "vector"
#include "bitset"
#include "sstream"
#include "fstream"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

using namespace std;
using namespace cv;

class bit_setter {
public:
    bit_setter(unsigned long total_bytes, vector<unsigned char> *bytes) : total_bytes(total_bytes - 1), bytes(bytes) {};

    Vec3b set_pixel_bits(Vec3b pixel) {
        bitset<8> red = bitset<8>(pixel[0]);
        bitset<8> green = bitset<8>(pixel[1]);
        bitset<8> blue = bitset<8>(pixel[2]);
        red = set_bit(red, (bitset<8>(bytes->at(current_byte))[current_bit]));
        if (all_data_embedded) return Vec3b(red.to_ulong(), green.to_ulong(), blue.to_ulong());
        green = set_bit(green, (bitset<8>(bytes->at(current_byte))[current_bit]));
        if (all_data_embedded) return Vec3b(red.to_ulong(), green.to_ulong(), blue.to_ulong());
        blue = set_bit(blue, (bitset<8>(bytes->at(current_byte))[current_bit]));
        return Vec3b(red.to_ulong(), green.to_ulong(), blue.to_ulong());
    }

    bool get_all_data_embedded() {
        return all_data_embedded;
    }

private:
    int current_bit = 0;
    unsigned long current_byte = 0, total_bytes = 0;;
    bool all_data_embedded = false;
    vector<unsigned char> *bytes = nullptr;

    void increment_bit() {
        current_bit++;
        if (current_bit == 8) {
            current_byte++;
            current_bit = 0;
            if (current_byte > total_bytes) all_data_embedded = true;
        }
    }

    bitset<8> set_bit(bitset<8> rgb_value, bool bit) {
        rgb_value.set(0, bit);
        increment_bit();
        return rgb_value;
    }
};

class bit_getter {
public:
    bit_getter(vector<unsigned char> *bytes) : bytes(bytes) {}

    bool retrieve_bits(Vec3b pixel) {
        bitset<8> red = bitset<8>(pixel[0]);
        bitset<8> green = bitset<8>(pixel[1]);
        bitset<8> blue = bitset<8>(pixel[2]);
        build_byte(red[0]);
        if (check_significant_bytes()) {return true;}
        build_byte(green[0]);
        if (check_significant_bytes()) {return true;}
        build_byte(blue[0]);
        if (check_significant_bytes()) {return true;}
        return false;
    }

    string get_file_name() {
        return file_name;
    }

    vector<unsigned char> get_file_data() {
        vector<unsigned char> file_data;
        for (size_t i = 9 + file_name_size; i < bytes->size(); i++) {
            file_data.push_back(bytes->at(i));
        }
        return file_data;
    }

private:
    bitset<8> building_byte;
    int current_bit = 0, file_name_size = 0;
    unsigned long current_byte = 0, file_size = 0;
    vector<unsigned char> *bytes = nullptr;
    bool all_data_retrieved = false, file_name_size_retrieved = false, file_size_retrieved = false, file_name_retrieved = false;
    string file_name;

    void build_byte(bool bit) {
        building_byte.set(current_bit, bit);
        current_bit++;
        if (current_bit == 8) {
            current_byte++;
            current_bit = 0;
            bytes->push_back(building_byte.to_ulong());
        }
    }

    bool check_significant_bytes() {
        if (bytes->size() == 1 && file_name_size_retrieved == false) {
            file_name_size = bytes->at(0);
            file_name_size_retrieved = true;
        }
        if (bytes->size() == static_cast<unsigned long>(1 + file_name_size) && file_name_retrieved == false) {
            for (int i = 1; i < 1 + file_name_size; i++) {
                file_name = file_name + static_cast<char>(bytes->at(i));
            }
            file_name_retrieved = true;
        }
        if (bytes->size() == static_cast<unsigned long>(9 + file_name_size) && file_size_retrieved == false) {
            bitset<64> bits;
            int count = 63;
            for (int i = 1 + file_name_size; i < 9 + file_name_size; i++) {
                bitset<8> single_byte = bytes->at(i);
                bits.set(count, single_byte[7]);
                count--;
                bits.set(count, single_byte[6]);
                count--;
                bits.set(count, single_byte[5]);
                count--;
                bits.set(count, single_byte[4]);
                count--;
                bits.set(count, single_byte[3]);
                count--;
                bits.set(count, single_byte[2]);
                count--;
                bits.set(count, single_byte[1]);
                count--;
                bits.set(count, single_byte[0]);
                count--;
            }
            file_size = bits.to_ulong();
            file_size_retrieved = true;
        }
        if (bytes->size() == file_size + file_name_size + 9) {
            all_data_retrieved = true;
            return true;
        }
        return false;
    }
};

void print_vec(vector<unsigned char> *bytes) {
    puts("Printing vector...");
    for (unsigned char byte : *bytes) {
        cout << bitset<8>(byte) << " ";
    }
    cout << endl;
}

streamsize get_file_size(ifstream *file) {
    file->seekg(0, ios::end);
    streamsize size = file->tellg();
    file->seekg(0, ios::beg);
    cout << "File size: " << size << endl;
    return size;
}

unsigned char get_byte(bitset<64> bits, int index) {
    bitset<8> byte;
    for (int i = 0; i < 8; i++) {
        byte[i] = bits[index];
        index++;
    }
    return byte.to_ulong();
}

void read_bits(vector<unsigned char> *bytes, ifstream *file, string file_name) {
    streamsize file_size = get_file_size(file);
    uint16_t file_name_size = file_name.length();

    bitset<8> file_name_size_bits = bitset<8>(file_name_size);
    bitset<64> file_size_bits = bitset<64>(file_size);

    char* buffer = new char[file_size];
    unsigned long current_byte = 0;
    bytes->push_back(file_name_size_bits.to_ulong());
    current_byte++;
    for (char c : file_name) {
        bytes->push_back(c);
        current_byte++;
    }
    {
        int count = 7;
        for (unsigned long i = current_byte; i < current_byte + 8; i++) {
            bytes->push_back(get_byte(file_size_bits, count * 8));
            count--;
        }
        current_byte = current_byte + count;
    }
    if (!file->read(buffer, file_size)) {
        puts("Error reading file");
    }
    for (long i = 0; i < file_size; i++) {
        bytes->push_back(buffer[i]);
    }
    delete buffer;
}

bool embed_file_into_image(Mat img, vector<unsigned char> *bytes) {
    bit_setter setter = bit_setter(bytes->size(), bytes);
    bool all_data_embedded = false;
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            Vec3b pixel = img.at<Vec3b>(i, j);
            auto new_pixel = setter.set_pixel_bits(pixel);
            img.at<Vec3b>(i, j) = new_pixel;
            if (setter.get_all_data_embedded()) {
                all_data_embedded = true;
                break;
            }
        }
        if (all_data_embedded) return true;
    }
    return false;
}

void write_data_to_file(bit_getter *getter) {
    ofstream file("RETRIEVED_" + getter->get_file_name(), ios::binary);
    if (!file) {
        puts("Error: Could not create file to write retrieved data to");
        exit(-1);
    }
    auto data = getter->get_file_data();
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
    cout << "Success! Retrieved data written to " << "RETRIEVED_" << getter->get_file_name()  << endl;
}

bool retrieve_file_from_image(Mat img, vector<unsigned char> *bytes) {
    bit_getter getter = bit_getter(bytes);
    bool all_data_retrieved = false;
    for (int i = 0; i < img.rows; i++ ) {
        for (int j = 0; j < img.cols; j++) {
            Vec3b pixel = img.at<Vec3b>(i, j);
            if (getter.retrieve_bits(pixel) == true) {
                all_data_retrieved = true;
                break;
            }
        }
        if (all_data_retrieved) {write_data_to_file(&getter); return true; break;}
    }
    return false;
}

int main(int argc, char *argv[]) {
    int flag = -1, action = -1;
    string file_name, message, image_name, destination;
    while ((flag = getopt(argc, argv, "ers:d:m:f:h")) != -1) {
        switch (flag) {
            case 'e': {
                if (action != -1) {puts("Error: Cannot both embed data and retrieve data in one command"); exit(-1);}
                action = 1;
                break;
            }
            case 'r': {
                if (action != -1) {puts("Error: Cannot both embed data and retrieve data in one command"); exit(-1);}
                action = 2;
                break;
            }
            case 's': {
                image_name = string(optarg);
                break;
            }
            case 'd': {
                destination = string(optarg);
                break;
            }
            case 'f': {
                file_name = string(optarg);
                break;
            }
            case 'm': {
                message = string(optarg);
                break;
            }
            case 'h': {
                puts("This steganography tool can embed a file into an image, or retrieve an embedded file from an image. Created images that contain a file will appear undistinguishable from the original image to the human eye.\n");
                puts("Command line flags:");
                puts("-e (Embed) This flag specifies that you want to embed a message into an image. No argument. WARNING! Due to embedding method, the resulting embedded image must be of a lossless file format. png is recommended.");
                puts("-r (Retrieve) This flag specifies that you want to retrieve a message from an image. No argument.");
                puts("-s (Source) This flag specifies the name of the image you want to embed a file in or retrieve a file from. Argument required");
                puts("-d (Destination) This flag specifies the name of embedded image that will contain the file. Argument required.");
                puts("-f (File) This flag specifies the name of the file that you want to embed within the image. Argument required.");
                puts("-h (Help) This flag prints out information regarding the use of the program. No argument");
                puts("");
                puts("Example of embedding a message\n./stegotool -e -f \"secret_file.txt\" -s \"source_image.png\" -d \"destination_image.png\"");
                puts("Example of retrieving a message\n./stegotool -r -s \"embedded_image.png\"");
                return 0;
                break;
            }
            case '?': {
                cout << "Error: Invalid flag: " << static_cast<char>(flag) << endl;
                break;
            }
        }
    }
    switch (action) {
        case 1: {
            if (!message.empty()) {

            }
            ifstream file(file_name, ios::binary);
            if (!file) {cout << "Error: Cannot open " << file_name << endl; exit(-1);}
            vector<unsigned char> bytes;
            read_bits(&bytes, &file, file_name);
            Mat img = imread(image_name, IMREAD_COLOR);
            if (!embed_file_into_image(img, &bytes)) {
                puts("Error: Image too small to store all data");
                exit(-1);
            }
            imwrite(destination, img);
            cout << "Success! Embedded image saved as " << destination << endl;
            break;
        }
        case 2: {
            Mat img = imread(image_name, IMREAD_COLOR);
            vector<unsigned char> bytes;
            if (!retrieve_file_from_image(img, &bytes)) {
                puts("Error: Data could not be retrieved from image");
                exit(-1);
            }
            break;
        }
        case -1: {
            puts("Error: Must specify if you want to embed or retrieve data");
            exit(-1);
        }
    }
}