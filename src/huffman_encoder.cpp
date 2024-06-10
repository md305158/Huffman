#include "../inc/huffman_encoder.h"

#include "../inc/bit_file_io.h"
#include "../inc/huffman_tree.h"
#include "../inc/ui.h"
#include <algorithm>
#include <climits>
#include <fstream>
#include <iostream>
#include <unordered_map>

static void write_file_header(const freq_map &map, uint8_t padding,
                              std::fstream &output_file, bit_file_io &wrapper);
static huffman_tree *read_file_header(std::fstream &file, bit_file_io &wrapper);

huffman_encoder::huffman_encoder(std::string input_file,
                                 std::string output_file, const ui &ui,
                                 const size_t buffer_size)
    : input_file_(std::move(input_file)), output_file_(std::move(output_file)),
      ui_(ui), buffer_(new uint8_t[buffer_size]), buffer_size_(buffer_size)
{
}

huffman_encoder::~huffman_encoder() { delete[] buffer_; }

void huffman_encoder::compress_file()
{
    this->ui_.write_message("Starting compression...");
    this->ui_.write_message("Output file: " + this->output_file_);
    std::fstream input_file(this->input_file_,
                            std::ios::in | std::ios_base::binary);

	//checking input and output files
    if (!input_file.good() ||
        input_file.peek() == std::ifstream::traits_type::eof())
    {
        input_file.close();
        this->ui_.app_error("Input file doesn't exists, or it's empty.");
        return;
    }

    std::fstream output_file(this->output_file_,
                             std::ios::out | std::ios_base::binary);
    if (!output_file.good())
    {
        input_file.close();
        output_file.close();
        this->ui_.app_error("Cannot create or write to output file.");
        return;
    }

    bit_file_io output_file_bit_io(output_file, 1, size_16_mb);

    this->ui_.write_message("Counting byte frequency...");

	//read and count bytes from file
    freq_map map;
    // https://stackoverflow.com/a/67854635

    while (input_file.good())
    {
        input_file.read(
            reinterpret_cast<char *>(this->buffer_),
            static_cast<std::streamsize>(sizeof(uint8_t) * this->buffer_size_));
        this->buffer_cnt_ = static_cast<size_t>(input_file.gcount());
        for (size_t i = 0; i < buffer_cnt_; i++)
        {
            uint8_t byte = this->buffer_[i];
            map.inc(byte);
        }
    }

    this->ui_.write_message("Finished counting bytes.");

    this->ui_.write_message("Building huffman tree...");
	//create a huffman tree and create codes for each byte
    const huffman_tree *tree = new huffman_tree(map);
    auto codes = tree->get_codes();
    this->ui_.write_message("Tree created, and codes generated.");

	//code may not be length mult of 8
	//in this case we should add padding before code
    // calculate needed padding
    uint8_t padding = 0;
    for (uint16_t i = 0; i <= UINT8_MAX; i++)
    {
		padding = (padding + ((codes[i].size() % CHAR_BIT) *
			(map.get(static_cast<uint8_t>(i)) % CHAR_BIT)) %
			CHAR_BIT) %
			CHAR_BIT;
    }


	padding = padding > 0 ? CHAR_BIT - padding : 0;
    this->ui_.write_message("Writing file header...");
	//create and write header to file
    write_file_header(map, padding, output_file, output_file_bit_io);
    this->ui_.write_message("File header written.");

    input_file.clear(); // clear eof flag
    input_file.seekg(0, std::ios_base::beg);

	//write code to file
    this->ui_.write_message("Encoding bytes...");
    while (input_file.good())
    {
        input_file.read(
            reinterpret_cast<char *>(this->buffer_),
            static_cast<std::streamsize>(sizeof(uint8_t) * this->buffer_size_));
        this->buffer_cnt_ = static_cast<size_t>(input_file.gcount());
        for (size_t i = 0; i < this->buffer_cnt_; i++)
        {
            uint8_t byte = this->buffer_[i];
            auto &code = codes[byte];
            for (bool bit : code)
                output_file_bit_io << (bit ? 1 : 0);
        }
    }

	//flush buffers
    output_file_bit_io.flush_bit_buffer();
    output_file_bit_io.flush_buffer();

    this->ui_.write_message("Compression finished");

    input_file.close();
    output_file.close();

    delete tree;
}

void huffman_encoder::decompress_file()
{
    this->ui_.write_message("Starting decompression...");

	//check input output files
    std::fstream input_file(this->input_file_,
                            std::ios::in | std::ios_base::binary);
    if (!input_file.good() ||
        input_file.peek() == std::ifstream::traits_type::eof())
    {
        input_file.close();
        this->ui_.app_error("Input file doesn't exists, or it's empty.");
        return;
    }
    bit_file_io input_file_bit_io(input_file, size_16_mb, 1);

    std::fstream output_file(this->output_file_,
                             std::ios::out | std::ios_base::binary);
    if (!output_file.good())
    {
        input_file.close();
        output_file.close();
        this->ui_.app_error("Cannot create or write to output file.");
        return;
    }

    this->ui_.write_message("Reading file header and rebuilding tree...");
	//read file header and construct tree from it
    const huffman_tree *tree;
    try
    {
        tree = read_file_header(input_file, input_file_bit_io);
    }
    catch (const std::logic_error &ex)
    {
        ui_.app_error(ex.what());
        return;
    }
    this->ui_.write_message("Tree created.");

    this->ui_.write_message("Transforming bytes...");
	//read code bit by bit, and assemble bytes
	//then write decompressed bytes to the output file
    uint8_t bit = 0, byte = 0;
    while (input_file_bit_io >> bit)
    {
        if (tree->try_get_byte(byte, bit))
        {
            this->buffer_[this->buffer_cnt_++] = byte;
            if (this->buffer_cnt_ == this->buffer_size_)
            {
                output_file.write(reinterpret_cast<char *>(this->buffer_),
                                  static_cast<std::streamsize>(
                                      sizeof(uint8_t) * this->buffer_size_));
                this->buffer_cnt_ = 0;
            }
        }
    }

	//flush buffer
    if (buffer_cnt_ > 0)
    {
        output_file.write(
            reinterpret_cast<char *>(this->buffer_),
            static_cast<std::streamsize>(sizeof(uint8_t) * this->buffer_cnt_));
        this->buffer_cnt_ = 0;
    }

    this->ui_.write_message("Decompression finished");
    input_file.close();
    output_file.close();

    delete tree;
}

static void write_file_header(const freq_map &map, const uint8_t padding,
                              std::fstream &output_file, bit_file_io &wrapper)
{
    uint8_t buf[2] = {
        static_cast<uint8_t>(map.size() -
                             1), // bytes_size + 1 = unique bytes count;
        padding                  // needed padding for huffman code
    };

    output_file.write(reinterpret_cast<char *>(&buf), sizeof(buf));

	//write bytes frequency
    for (uint16_t chr = 0; chr <= UINT8_MAX; chr++)
    {
        if (uint64_t frq = map.get(static_cast<uint8_t>(chr)))
        {
            output_file.write(reinterpret_cast<char *>(&chr), sizeof(uint8_t));
            output_file.write(reinterpret_cast<char *>(&frq), sizeof(uint64_t));
        }
    }

	//add padding bytes
    for (uint8_t i = 0; i < padding; i++)
        wrapper << 0;
    // no need to flush it here
    // flushing will cause problems
	// because we will write more zeros
}

static huffman_tree *read_file_header(std::fstream &file, bit_file_io &wrapper)
{
    uint8_t header[2];
    // header[0] -> num of unique bytes - 1
    // header[1] -> code padding

	//read header to header array
    file.read(reinterpret_cast<char *>(&header), sizeof(header));
    const uint16_t unique_bytes = static_cast<uint16_t>(header[0]) + 1;
    freq_map map;

	uint8_t byte = 0;
    uint64_t count = 0;

	//how many bytes are read
    uint16_t bytes_read = 2;

	//read bytes frequency
    while (bytes_read < ((unique_bytes * 9) + 2) &&
           file.read(reinterpret_cast<char *>(&byte), sizeof(uint8_t)) &&
           file.read(reinterpret_cast<char *>(&count), sizeof(uint64_t)))
    {
        bytes_read += 9;
        map.set(byte, count);
    }

	//read padding bits
    uint8_t bit;
    for (uint8_t i = 0; i < header[1] && wrapper >> bit; i++)
        ;

	//construct tree
    return new huffman_tree(map);
}
