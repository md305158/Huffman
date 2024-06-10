#pragma once

#include <cstdint>
#include <fstream>

/**
 * @brief Klasa opakowująca std::fstream. Umożliwia pisanie i czytanie, z i do
 * pliku, pojedyczych bitów
 */
class bit_file_io
{
  private:
    std::fstream &file_stream_;
    // buffers for bit manipulation
    uint8_t r_bit_buf_ = 0, r_bit_buf_size_ = 0;
    uint8_t w_bit_buf_ = 0, w_bit_buf_size_ = 0;

    // buffers for reading/writing from/to file
    uint8_t *r_buff_;
    size_t r_buff_cnt_ = 0, r_buff_size_;
    uint8_t *w_buff_;
    size_t w_buff_cnt_ = 0, w_buff_size_;

  public:
    bit_file_io(std::fstream &file_stream, size_t read_buff_size,
                size_t write_buff_size);
    ~bit_file_io();

    /**
     * @brief Pisze pojedynczy bit do pliku.
     * @param bit - bit który zostanie wypisany do pliku
     */
    void write_bit(uint8_t bit);

    /**
     * @brief Powoduje wypisanie i wyczyszczenie buforu bitów
     */
    void flush_bit_buffer();

    /**
     * @brief Powoduje wypisanie i wyczyszczenie buforu
     */
    void flush_buffer();

    /**
     * @brief Przeczytaj pojedynczy bit z pliku
     * @param[out] bit - bit który zostanie przeczytany
     * @return true - kiedy bit został przeczytany
     * @return false - kiedy nie ma już bitów do przeczytania
     */
    bool read_bit(uint8_t &bit);

    bit_file_io &operator<<(const uint8_t bit);
    bool operator>>(uint8_t &bit);
};
