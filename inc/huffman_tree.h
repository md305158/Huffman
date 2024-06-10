#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

/**
 * @brief Klasa pomocnicza do przechowywania częstotliwości wystepowania bajtów
 */
class freq_map
{
  private:
    std::vector<uint64_t> freq_;

  public:
    freq_map() : freq_(std::vector<uint64_t>(UINT8_MAX + 1, 0)) {}

    /**
     * @brief Pobiera ilość występowania danego bajtu
     *
     * @param byte bajt
     * @return uint64_t częstotliwość bajtu byte
     */
    uint64_t get(uint8_t byte) const { return freq_[byte]; }

    /**
     * @brief Ustawia ilość wystepowania danego bajtu na podaną wartość
     *
     * @param byte bajt
     * @param value nowa ilość
     */
    void set(uint8_t byte, uint64_t value) { freq_[byte] = value; }

    /**
     * @brief Inkrementuje częstotliwość danego bajtu
     *
     * @param byte byte
     */
    void inc(uint8_t byte) { ++freq_[byte]; }

    /**
     * @brief Zwraca ilość unikatowych bajtów
     * @return uint16_t - ilość unikatowych bajtów
     */
    uint16_t size() const
    {
        uint16_t cnt = 0;
        for (uint16_t i = 0; i <= UINT8_MAX; i++)
            if (freq_[i])
                cnt++;
        return cnt;
    };
};

/**
 * @brief Wierzchołek drzewa Huffmana
 */
class huffman_node
{
  private:
    uint64_t freq_ = 0;
    huffman_node *left_node_{}, *right_node_{};

  public:
    huffman_node(const uint64_t frequency, huffman_node *left_child,
                 huffman_node *right_child)
        : freq_(frequency), left_node_(left_child), right_node_(right_child)
    {
    }
    virtual ~huffman_node() = default;

    /**
     * @brief Pobiera częstotlowść przechowywaną przez wierzchołek
     * @return uint64_t - częstotliwość
     */
    uint64_t get_frequency() const { return this->freq_; }

    /**
     * @brief Zwraca lewe dziecko
     * @return huffman_node* - lewe dziecko
     */
    huffman_node *get_left_child() const { return this->left_node_; }

    /**
     * @brief Zwraca prawe dziecko
     * @return huffman_node* - prawe dziecko
     */
    huffman_node *get_right_child() const { return this->right_node_; }
};

/**
 * @brief Liść drzewa Huffmana. Zawiera częstotliwość oraz bajt, który
 * reprezentuje. Dziedziczy po huffman_node
 */
class huffman_leaf : public huffman_node
{
  private:
    uint8_t value_ = 0;

  public:
    huffman_leaf(const uint64_t frequency, const uint8_t value)
        : huffman_node(frequency, nullptr, nullptr), value_(value)
    {
    }

    /**
     * @brief Zwraca bajt reprezentowany przez liść
     * @return uint8_t - bajt reprezentowany przez liść
     */
    uint8_t get_value() const { return this->value_; }
};

/**
 * @brief Reprezentacja drzewa Huffmana
 */
class huffman_tree
{
  private:
    huffman_node *tree_root_ = nullptr;
    const freq_map &chars_freq_;
    std::vector<uint8_t> *codes_;
    void fill_codes(huffman_node *root, std::vector<uint8_t> current);

  public:
    /**
     * @brief Tworzy drzewo Huffmana oraz generuje kod dla każdego bajtu, przy
     * użyciu podanych częstotliwości bajtów
     *
     * @param[in] chars_freq - struktura zawierająca częstotliwość bajtów
     */
    huffman_tree(const freq_map &chars_freq);
    ~huffman_tree();

    /**
     * @brief Zwraca listę kodów
     *
     * @return const std::vector<uint8_t>* wskażnik do listy kodów
     */
    const std::vector<uint8_t> *get_codes() const { return this->codes_; }

    /**
     * @brief Przy użyciu statycznego bufora próbuje odczytać bajt z podanego
     * kodu. Jeżeli kod nie jest jeszcze jednoznaczny funkcja dopisuje bit do
     * bufora i zwraca false. W przeciwnym wypadku czyściu bufor, ustawia byte
     * na odpowiedni bajt i zwraca true
     *
     * @param[out] byte bajt
     * @param code_bit bit kodu
     * @return true - jeżeli bajt został odczytany
     * @return false - jeżeli kod jest jeszcze niejednoznaczny
     */
    bool try_get_byte(uint8_t &byte, uint8_t code_bit) const;
};
