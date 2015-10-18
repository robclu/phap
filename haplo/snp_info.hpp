// ----------------------------------------------------------------------------------------------------------
/// @file   Header file for parahaplo snp info class
// ----------------------------------------------------------------------------------------------------------

#ifndef PARHAPLO_SNP_INFO_HPP
#define PARHAPLO_SNP_INFO_HPP

#include <stdint.h>

namespace haplo {
           
// ----------------------------------------------------------------------------------------------------------
/// @class      SnpInfo
/// @brief      Stores some information about a SNP
// ----------------------------------------------------------------------------------------------------------
class SnpInfo {
private:
    size_t      _start_idx;
    size_t      _end_idx;
    size_t      _zeros;
    size_t      _ones;
    uint8_t     _type;          //!< IH or NIH
public:
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Constructor
    // ------------------------------------------------------------------------------------------------------
    SnpInfo() noexcept 
    : _start_idx{0}, _end_idx{0}, _zeros{0}, _ones{0}, _type{0} {}

    // ------------------------------------------------------------------------------------------------------
    /// @brief      Constructor -- sets the values
    /// @param[in]  start_index     The start index (column) of the read
    /// @param[in]  end_index       The end index of the read
    // ------------------------------------------------------------------------------------------------------
    SnpInfo(const size_t start_idx, const size_t end_idx) noexcept
    : _start_idx(start_idx), _end_idx(end_idx), _zeros{0}, _ones{0}, _type{0} {}
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets the staet index of the read 
    // ------------------------------------------------------------------------------------------------------
    inline size_t start_index() const { return _start_idx; }
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets the staet index of the read 
    // ------------------------------------------------------------------------------------------------------
    inline size_t& start_index() { return _start_idx; }
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets the end index of the read
    // ------------------------------------------------------------------------------------------------------
    inline size_t end_index() const { return _end_idx; }

    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets the end index of the read
    // ------------------------------------------------------------------------------------------------------
    inline size_t& end_index() { return _end_idx; }
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets thenumber of zeros
    // ------------------------------------------------------------------------------------------------------
    inline size_t zeros() const { return _zeros; }

    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets thenumber of zeros
    // ------------------------------------------------------------------------------------------------------
    inline size_t& zeros() { return _zeros; }
    // ------------------------------------------------------------------------------------------------------
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets thenumber of ones
    // ------------------------------------------------------------------------------------------------------
    inline size_t ones() const { return _ones; }

    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets thenumber of ones
    // ------------------------------------------------------------------------------------------------------
    inline size_t& ones() { return _ones; }
    
    // ------------------------------------------------------------------------------------------------------
    /// @brief      Sets the type of the snp
    /// @param[in]  value   The value to set the type to
    // ------------------------------------------------------------------------------------------------------
    inline void set_type(const uint8_t value) { _type = value & 0x03; }

    // ------------------------------------------------------------------------------------------------------
    /// @brief      Gets the type of the snp
    // ------------------------------------------------------------------------------------------------------
    inline size_t type() const { return _type; }    

    // ------------------------------------------------------------------------------------------------------
    /// @brief      Returns true of the snp is monotone (contains only 0's or 1's)
    // ------------------------------------------------------------------------------------------------------
    inline bool is_monotone() const { return ((_ones > 0 && _zeros == 0) || (_zeros > 0 && _ones == 0)); } 

    /// @brief      Gets the length of the read 
    // ------------------------------------------------------------------------------------------------------
    inline size_t length() const { return _end_idx - _start_idx + 1; }
};

}           // End namespace haplo
#endif      // PARAHAPLO_SNP_INFO_HPP
