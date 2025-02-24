🖼️ Grayscale Image Compression
Grayscale Image Compression is a C++ console application that compresses, decompresses, and calculates the compression rate of grayscale images using lossless compression algorithms, specifically Huffman coding and Lempel-Ziv-Welch (LZW). This project was developed using OpenCV for image processing.

🚀 Features
Image Compression: Compress grayscale BMP images without quality loss.
Decompression: Reconstruct original images from compressed binary files.
Compression Rate Calculation: Evaluate the efficiency of the compression.
User-Friendly Console: Interactive CLI for easy command execution.

🛠️ Technologies Used
Language: C++
Image Processing Library: OpenCV
Algorithms: Huffman Coding and Lempel-Ziv-Welch (LZW)

📄 How It Works

Huffman Coding
Calculates pixel probabilities.
Builds a Huffman tree based on frequency.
Encodes frequent symbols with shorter codes.
Generates a compressed binary file and reconstructs the image upon decompression.

Lempel-Ziv-Welch (LZW)
Iteratively builds a dictionary of pixel sequences.
Encodes the longest matching sequence found in the dictionary.
Efficiently compresses images without requiring the dictionary for decompression.
