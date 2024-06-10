#include "../inc/huffman_tree.h"

#include <queue>
#include <stdexcept>
#include <utility>

static void delete_tree(const huffman_node *root);
static huffman_leaf *get_left_most_leaf(huffman_node *node);

//constructs huffman tree from bytes frequency
huffman_tree::huffman_tree(const freq_map &chars_freq) : chars_freq_(chars_freq)
{
    // https://stackoverflow.com/a/5808171
    // min heap comparator
    auto comp = [](huffman_node *n1, huffman_node *n2)
    {
        const uint64_t freq1 = n1->get_frequency();
        const uint64_t freq2 = n2->get_frequency();

        // if booth of nodes have equal frequency and are leafs, choose one with
        // smaller value
        if (freq1 == freq2)
        {
            const huffman_leaf *leaf1 = dynamic_cast<huffman_leaf *>(n1),
                               *leaf2 = dynamic_cast<huffman_leaf *>(n2);
            if (leaf1 != nullptr && leaf2 != nullptr)
            {
                return leaf1->get_value() > leaf2->get_value();
            }
            if (leaf1 != nullptr)
            {
                return false;
            }
            if (leaf2 != nullptr)
            {
                return true;
            }
            leaf1 = get_left_most_leaf(n1);
            leaf2 = get_left_most_leaf(n2);
            return leaf1->get_value() > leaf2->get_value();
        }
        return freq1 > freq2;
    };
    std::priority_queue<huffman_node *, std::vector<huffman_node *>,
                        decltype(comp)>
	pq(comp);

    for (uint16_t chr = 0; chr <= UINT8_MAX; chr++)
        if (const uint64_t freq =
                this->chars_freq_.get(static_cast<uint8_t>(chr)))
            pq.push(new huffman_leaf(freq, static_cast<uint8_t>(chr)));

    if (pq.empty())
        throw std::logic_error("Cannot create tree with 0 unique bytes.");

    if (pq.size() == 1)
    {
        const auto node = pq.top();
        pq.pop();
        pq.push(new huffman_node(node->get_frequency(), node, nullptr));
    }

    while (pq.size() > 1)
    {
        auto node1 = pq.top();
        pq.pop();
        auto node2 = pq.top();
        pq.pop();

        auto parent = new huffman_node(
            node1->get_frequency() + node2->get_frequency(), node1, node2);
        pq.push(parent);
    }

    tree_root_ = pq.top();
    pq.pop();

    codes_ = new std::vector<uint8_t>[UINT8_MAX + 1];
    fill_codes(this->tree_root_, {});
}

huffman_tree::~huffman_tree()
{
    delete_tree(this->tree_root_);
    delete[] codes_;
}

bool huffman_tree::try_get_byte(uint8_t &byte, uint8_t code_bit) const
{
    static huffman_node *current_node = this->tree_root_;
    if (code_bit)
        current_node = current_node->get_right_child();
    else
        current_node = current_node->get_left_child();

    if (const auto leaf = dynamic_cast<huffman_leaf *>(current_node))
    {
        byte = leaf->get_value();
        current_node = this->tree_root_;
        return true;
    }
    return false;
}

//fill codes array with corresponding codes
//code_[x] -> huffman code for byte x
void huffman_tree::fill_codes(huffman_node *root, std::vector<uint8_t> current)
{
    if (root == nullptr)
        return;

    if (const auto leaf = dynamic_cast<huffman_leaf *>(root))
    {
        this->codes_[leaf->get_value()] = std::move(current);
        return;
    }

    auto cpy = std::vector<uint8_t>(current);
    current.push_back(0);
    fill_codes(root->get_left_child(), current);
    cpy.push_back(1);
    fill_codes(root->get_right_child(), cpy);
}

//cleanup tree
static void delete_tree(const huffman_node *root)
{
    if (root == nullptr)
        return;
    if (root->get_left_child() != nullptr)
        delete_tree(root->get_left_child());
    if (root->get_right_child() != nullptr)
        delete_tree(root->get_right_child());
    delete root;
}

static huffman_leaf *get_left_most_leaf(huffman_node *node)
{
    if (node == nullptr)
    {
        throw std::logic_error("Unknown error. Cannot create tree");
    }

    huffman_leaf *leaf;
    while ((leaf = dynamic_cast<huffman_leaf *>(node)) == nullptr)
    {
        node = node->get_left_child();
    }
    return leaf;
}
