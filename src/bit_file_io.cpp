#include "../inc/bit_file_io.h"

#include <climits>

bit_file_io::bit_file_io(std::fstream &file_stream, size_t read_buff_size,
                         size_t write_buff_size)
    : file_stream_(file_stream),
      r_buff_size_(std::max(read_buff_size, static_cast<size_t>(1))),
      w_buff_size_(std::max(write_buff_size, static_cast<size_t>(1)))
{
    r_buff_ = new uint8_t[r_buff_size_];
    w_buff_ = new uint8_t[w_buff_size_];
}

bit_file_io::~bit_file_io()
{
    delete r_buff_;
    delete w_buff_;
}

void bit_file_io::write_bit(uint8_t bit)
{
    bit &= 1;

    if (bit)
        this->w_bit_buf_ |= 1 << ((CHAR_BIT - 1) - this->w_bit_buf_size_);
    this->w_bit_buf_size_++;

    if (this->w_bit_buf_size_ == CHAR_BIT)
        flush_bit_buffer();
}

/**
 * @brief Causes w_bit_buf_ to be saved to w_buff_. If w_buff_ achieves max size
 * it's being flushed
 */
void bit_file_io::flush_bit_buffer()
{
    if (this->w_bit_buf_size_ == 0)
        return;

    this->w_buff_[this->w_buff_cnt_++] = this->w_bit_buf_;
    this->w_bit_buf_size_ = 0;
    this->w_bit_buf_ = 0;

    if (this->w_buff_cnt_ == this->w_buff_size_)
        flush_buffer();
}

/**
 * @brief Causes w_buff_ to be written to file. Any data in w_bit_buf_ is
 * ignored
 */
void bit_file_io::flush_buffer()
{
    if (this->w_buff_cnt_ == 0)
        return;

    file_stream_.write(reinterpret_cast<char *>(this->w_buff_),
                       sizeof(uint8_t) * this->w_buff_cnt_);
    this->w_buff_cnt_ = 0;
}

/**
 * @brief Reads single bit from file
 */
bool bit_file_io::read_bit(uint8_t &bit)
{
	if (this->r_bit_buf_size_ == 0)
    {
	    static size_t r_buff_pos = 0;
	    if (r_buff_pos == this->r_buff_cnt_)
        {
			this->file_stream_.read(
				reinterpret_cast<char*>(this->r_buff_),
				static_cast<std::streamsize>(sizeof(uint8_t) * this->r_buff_size_));
            this->r_buff_cnt_ = this->file_stream_.gcount();
			if (this->r_buff_cnt_ == 0) 
				return false;
			r_buff_pos = 0;
        }

        this->r_bit_buf_ = this->r_buff_[r_buff_pos++];
        this->r_bit_buf_size_ = CHAR_BIT;
    }

    this->r_bit_buf_size_--;
    bit = (r_bit_buf_ & (1 << this->r_bit_buf_size_)) >> this->r_bit_buf_size_;
    return true;
}

bit_file_io &bit_file_io::operator<<(const uint8_t bit)
{
    this->write_bit(bit);
    return *this;
}

bool bit_file_io::operator>>(uint8_t &bit) { return this->read_bit(bit); }
