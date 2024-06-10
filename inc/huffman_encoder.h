#pragma once

#include <cstdint>
#include <string>

#include "consts.h"
#include "ui.h"

/**
 * @brief Klasa służąca do kompresji/dekompresji plików przy pomocy kodowania Huffmana
 */
class huffman_encoder
{
  private:
    std::string input_file_, output_file_;
    const ui &ui_;

    uint8_t *const buffer_;
    const size_t buffer_size_;
    size_t buffer_cnt_ = 0;

  public:
	/**
	 * @brief Tworzy nowy obiekt encodera
	 * 
	 * @param input_file - ścierzka do pliku wejściowego
	 * @param output_file - ścierzka do pliku wyjściowego
	 * @param ui - implementacja interfejsu użytkownika
	 * @param buffer_size - rozmiar wewnętrznego bufora
	 */
    huffman_encoder(std::string input_file, std::string output_file,
                    const ui &ui, const size_t buffer_size = size_16_mb);
    ~huffman_encoder();

	/**
	 * @brief Funkcja kompresująca plik
	 */
    void compress_file();
	/**
	 * @brief Funkcja dekompresująca plik
	 */
    void decompress_file();
};
