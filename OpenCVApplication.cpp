
#include "stdafx.h"
#include "common.h"
#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <fstream>
#include <vector>

using namespace std;
using std::vector;

struct HuffmanNode{
    int frequency;
    int value;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(int freq, uchar val) : frequency(freq), value(val), left(nullptr), right(nullptr) {}
};

struct CompareHuffmanNodes{
    bool operator()(const HuffmanNode* a, const HuffmanNode* b) const{
        return a->frequency > b->frequency;
    }
};

void assignBinaryCodes(HuffmanNode* node, string code, vector<string>& codes)
{
    if (node == nullptr)
    {
        return;
    }
    if (node->left == nullptr && node->right == nullptr)
    {
        codes[node->value] = code;
    }
    assignBinaryCodes(node->left, code + "0", codes);
    assignBinaryCodes(node->right, code + "1", codes);
}

HuffmanNode* buildHuffmanTree(vector<int> frequencyVector) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompareHuffmanNodes> priorityQueue;
    for (int i = 0; i < frequencyVector.size(); i++){
        if (frequencyVector[i] > 0) {
            priorityQueue.push(new HuffmanNode(frequencyVector[i], i));
        }
    }
    while (priorityQueue.size() > 1)
    {
        HuffmanNode* left = priorityQueue.top();
        priorityQueue.pop();
        HuffmanNode* right = priorityQueue.top();
        priorityQueue.pop();

        HuffmanNode* parent = new HuffmanNode(left->frequency + right->frequency, 0);
        parent->left = left;
        parent->right = right;

        priorityQueue.push(parent);
    }

    HuffmanNode* root = priorityQueue.top();
    priorityQueue.pop();

    return root;
}

vector<int> createFrequencyVector(vector<int> pixel_data) {

    vector<int> frequencyVector(256);

    for (int i = 0; i < pixel_data.size() - 1; i++) {
        frequencyVector[pixel_data[i]] ++;
    }

    return frequencyVector;
}

vector<int> createPixelVector(Mat_<uchar> img) {

    int index = 0;
    int num_pixels = img.rows * img.cols;
    vector<int> pixel_data(num_pixels);

    for (int i = 0; i < img.rows; i++){
        for (int j = 0; j < img.cols; j++){
            pixel_data[index] = img(i, j);
            index++;
        }
    }

    return pixel_data;
}

void writeDataToBinaryFile(const string& filename, vector<string> codes, Mat_<uchar> img) {
    ofstream outfile(filename, ios::out, ios::binary);
    outfile.close();
    outfile.open(filename, ios::out | ios::binary | ios::trunc);

    if (!outfile.is_open()) {
        cout << "Failed to open file!\n";
        return;
    }

    outfile.write(reinterpret_cast<const char*>(&img.rows), sizeof(img.rows));
    outfile.write(reinterpret_cast<const char*>(&img.cols), sizeof(img.cols));

    for (int i = 0; i < codes.size(); i++) {

        string code = codes[i];
        int index = i;
        unsigned char sizeChar = static_cast<unsigned char>(code.length());
        unsigned char ucharValue = static_cast<unsigned char>(index);
        outfile.write(reinterpret_cast<const char*>(&index), sizeof(unsigned char));
        outfile.write(reinterpret_cast<const char*>(&sizeChar), sizeof(unsigned char));

        unsigned char byte = 0;
        int bitCount = 0;

        for (char bit : code) {
            byte <<= 1;
            if (bit == '1') {
                byte |= 1;
            }
            bitCount++;

            if (bitCount == 8) {
                outfile.write(reinterpret_cast<const char*>(&byte), sizeof(unsigned char));
                byte = 0;
                bitCount = 0;
            }
        }

        if (bitCount > 0) {
            byte <<= (8 - bitCount);
            outfile.write(reinterpret_cast<const char*>(&byte), sizeof(unsigned char));
        }
    }

    int bitCounter = 0;
    uchar bitBuffer = 0;

    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {

            string codification = codes[img(i, j)];
            for (char bit : codification) {
                bitBuffer >>= 1; 

                if (bit == '1') {
                    bitBuffer |= (1 << 7); 
                }

                bitCounter++;

                if (bitCounter == 8) {
                    outfile.write(reinterpret_cast<const char*>(&bitBuffer), sizeof(unsigned char));
                    bitBuffer = 0;
                    bitCounter = 0;
                }
            }
        }
    }

    if (bitCounter < 8) {
        while (bitCounter < 8) {
            bitBuffer >>= 1;
            bitCounter++;
        }
        outfile.write(reinterpret_cast<const char*>(&bitBuffer), sizeof(unsigned char));
    }

    outfile.close();    
}

string bytesToBinaryString(const unsigned char codeBytes[], int length) {
    string binaryString;
    for (int i = 0; i < length; i++) {
        unsigned char byte = codeBytes[i];
        for (int j = 7; j >= 0; j--) {
            binaryString += ((byte >> j) & 1) ? "1" : "0";
        }
    }
    return binaryString;
}

int checkIfCodeExists(vector<string> codes, string code) {
    for (int i = 0; i < codes.size(); i++) {
        if (codes[i] == code) {
            return i;
        }
    }
    return 0;
}

Mat_<uchar> readHeaderDataFromBinaryFile(const string& filename) {
    ifstream infile(filename, ios::in | ios::binary);

    if (!infile.is_open()) {
        cout << "Failed to open file!\n";
        exit(-1);
    }

    int rows;
    int cols;
    vector<string> codes(256);

    infile.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    infile.read(reinterpret_cast<char*>(&cols), sizeof(cols));

    for (int i = 0; i < codes.size(); i++) {
        unsigned char ucharValue;
        unsigned char sizeChar;
        string code;

        infile.read(reinterpret_cast<char*>(&ucharValue), 1);
        infile.read(reinterpret_cast<char*>(&sizeChar), sizeof(unsigned char));

        int index = static_cast<int>(ucharValue);
        int length = static_cast<int>(sizeChar);
        int byteLength = (length % 8 != 0) ? ((length + 7) / 8) : (length / 8);
        unsigned char* codeBytes = new unsigned char[byteLength];

        for (int b = 0; b < byteLength; b++) {
            infile.read(reinterpret_cast<char*>(&codeBytes[b]), sizeof(unsigned char));
        }

        code = bytesToBinaryString(codeBytes, byteLength);

        if (code.length() > length) {
            code = code.substr(0, length);
        }

        codes[index] = code;

        delete[] codeBytes;
    }

    Mat_<uchar> img(rows, cols);
    string imageEncoded = "";
    uchar currentByte = 0;

	while (infile.read(reinterpret_cast<char*>(&currentByte), 1)) {
		for (int j = 0; j < 8; j++) {
			if (currentByte & (1 << j)) {
				imageEncoded.append("1");
			}
			else {
				imageEncoded.append("0");
			}
		}
	}


    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {

                string codification = "";
                int lenght = 0;

                for (char c : imageEncoded) {

                    codification += c;
                    lenght += 1;

                    int pixel_value = checkIfCodeExists(codes, codification);
                    if (pixel_value != 0) {
                        img(i, j) = (uchar)pixel_value;
                        imageEncoded = imageEncoded.substr(lenght);
                        break;
                    }
                }
        }
    }

    return img;

    infile.close();
}

void encodeData(string image_name) {
    string image_path = "Images/";
    image_path.append(image_name);
    image_path.append(".bmp");
     
    string compressed_file_path = "Compressed/";
    compressed_file_path.append(image_name);
    compressed_file_path.append(".bin");

    Mat_<uchar> img = imread(image_path, IMREAD_GRAYSCALE);

    imshow("Original", img);
    waitKey();

    if (img.empty())
    {
        cout << "Failed to load image." << endl;
        return;
    }

    vector<int> pixelData = createPixelVector(img);
    vector<int> frequencyVector = createFrequencyVector(pixelData);
    HuffmanNode* root = buildHuffmanTree(frequencyVector);      
    
    vector<string> codes(256);
    assignBinaryCodes(root, "", codes);

    writeDataToBinaryFile(compressed_file_path, codes, img);
}

void decodeData(string image_name) {
    string compressed_file_path = "Compressed/";
    compressed_file_path.append(image_name);
    compressed_file_path.append(".bin");

    string decompressed_file_path = "Decompressed/";
    decompressed_file_path.append(image_name);
    decompressed_file_path.append(".bmp");

    Mat_<uchar> img = readHeaderDataFromBinaryFile(compressed_file_path);

    imwrite(decompressed_file_path, img);
    imshow("Decompressed", img);
    waitKey();
}

int getFileSize(const string& file_path) {
    
    ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        cerr << "Error opening file: " << file_path << endl;
        return -1;
    }

    streamsize size = file.tellg();
    file.close();
    return static_cast<int>(size);
}

void computeCompressionRate(string image_name) {
    string image_path = "Images/";
    image_path.append(image_name);
    image_path.append(".bmp");

    string compressed_file_path = "Compressed/";
    compressed_file_path.append(image_name);
    compressed_file_path.append(".bin");

    int image_size = getFileSize(image_path);
    if (image_size != -1) {
        cout << "Image size: " << image_size << " bytes" << endl;
    }

    int compressed_file_size = getFileSize(compressed_file_path);
    if (compressed_file_size != -1) {
        cout << "Compressed file size: " << compressed_file_size << " bytes" << endl;
    }

    double compression_rate = static_cast<double>(image_size - compressed_file_size) / image_size;
    compression_rate *= 100; 

    cout << "Compression rate: " << compression_rate << "%" << std::endl;
}

int main() {
    int command;
    string userInput;

    while (true) {
        cout << "Menu:" << endl;
        cout << "1. Compress an Image" << endl;
        cout << "2. Decompress an Image" << endl;
        cout << "3. Test" << endl;
        cout << "4. Calculate Compression Rate" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter your command: ";
        cin >> command;

        switch (command) {
        case 1:
            cout << "Enter a string: ";
            cin.ignore(); 
            getline(cin, userInput);
            cout << "You entered: " << userInput << endl;
            encodeData(userInput);
            break;
        case 2:
            cout << "Enter a string: ";
            cin.ignore(); 
            getline(cin, userInput);
            cout << "You entered: " << userInput << endl;
            decodeData(userInput);
            break;
        case 3:
            cout << "Enter a string: ";
            cin.ignore();
            getline(cin, userInput);
            cout << "You entered: " << userInput << endl;
            encodeData(userInput);
            decodeData(userInput);
            break;
        case 4:
            cout << "Enter a string: ";
            cin.ignore();
            getline(cin, userInput);
            cout << "You entered: " << userInput << endl;
            computeCompressionRate(userInput);
            break;
        case 0:
            cout << "Exiting..." << endl;
            return 0;
        default:
            cout << "Invalid command! Please try again." << endl;
        }
    }

    return 0;
}
