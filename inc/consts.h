#pragma once

#include <cstddef>

/**
 * @brief Makro do celowego ignorowania ostrzeżenia o nieużywanej zmiennej
 */
#define UNUSED(x) (void)x

/**
 * @brief Długość bufora bajtów o rozmiarze 16 mb
 */
static constexpr size_t size_16_mb = 16777216; // 16mb

/**
 * @brief Długość bufora bajtów o rozmiarze 8 mb
 */
static constexpr size_t size_8_mb = 8388608; // 8mb
