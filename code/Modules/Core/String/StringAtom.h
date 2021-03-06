#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::StringAtom
    @ingroup Core
    @brief immutable, unique strings for fast comparison
    
    A unique string, relatively slow on creation, but fast for comparison.
    String atoms are stored in thread-local stringAtomTables and comparison
    is fastest in the creator thread.
    
    @see String
*/
#include "Core/Types.h"
#include "Core/String/stringAtomTable.h"

namespace Oryol {

class String;

class StringAtom {
public:
    /// default constructor
    StringAtom();
    /// construct from String
    StringAtom(const String& str);
    /// construct from raw string (slow)
    StringAtom(const char* str);
    /// construct from raw string (slow)
    StringAtom(const uchar* str);
    /// copy-constructor (fast if rhs was created in same thread)
    StringAtom(const StringAtom& rhs);
    /// move-constructor
    StringAtom(StringAtom&& rhs);
    /// destructor
    ~StringAtom();
    
    /// copy assignment
    void operator=(const StringAtom& rhs);
    /// move assignment
    void operator=(StringAtom&& rhs);
    /// assign raw string (slow)
    void operator=(const char* rhs);
    /// assign raw string (slow)
    void operator=(const uchar* rhs);
    /// assign from String object (slow)
    void operator=(const String& rhs);
    
    /// equality operator (FAST)
    bool operator==(const StringAtom& rhs) const;
    /// inequality operator (FAST)
    bool operator!=(const StringAtom& rhs) const;
    /// less-than operator (for storing in sets and maps)
    bool operator<(const StringAtom& rhs) const;
    
    /// equality operator with raw string (SLOW!)
    bool operator==(const char* rhs) const;
    /// inequality operator with raw string (SLOW!)
    bool operator!=(const char* rhs) const;
    /// equality operator with raw string (SLOW!)
    bool operator==(const uchar* rhs) const;
    /// inequality operator with raw string (SLOW!)
    bool operator!=(const uchar* rhs) const;
    /// equality operator with String (SLOW!)
    bool operator==(const String& rhs) const;
    /// inequality operator with String (SLOW!)
    bool operator!=(const String& rhs) const;
    
    /// clear content (becomes invalid)
    void Clear();
    /// return true if valid (contains a non-empty string)
    bool IsValid() const;
    /// return true if empty (contains no string, or empty string)
    bool Empty() const;
    /// get length
    int32 Length() const;
    /// get contained c-string
    const char* AsCStr() const;
    /// get String (slow because string object must be constructed)
    String AsString() const;

private:
    /// copy content
    void copy(const StringAtom& rhs);
    /// setup from C string
    void setupFromCString(const char* str);
    
    const stringAtomBuffer::Header* data;
    static const char* emptyString;
};

//------------------------------------------------------------------------------
inline void
StringAtom::Clear() {
    this->data = nullptr;
}

//------------------------------------------------------------------------------
inline
StringAtom::StringAtom()
    : data(nullptr) {
    // empty
}

//------------------------------------------------------------------------------
inline
StringAtom::StringAtom(const char* rhs) {
    this->setupFromCString(rhs);
}

//------------------------------------------------------------------------------
inline
StringAtom::StringAtom(const uchar* rhs) {
    this->setupFromCString((const char*) rhs);
}

//------------------------------------------------------------------------------
inline
StringAtom::StringAtom(const StringAtom& rhs) {
    this->copy(rhs);
}

//------------------------------------------------------------------------------
inline
StringAtom::StringAtom(StringAtom&& rhs) {
    this->copy(rhs);
    rhs.data = nullptr;
}

//------------------------------------------------------------------------------
inline
StringAtom::~StringAtom() {
    this->Clear();
}

//------------------------------------------------------------------------------
inline void
StringAtom::operator=(const StringAtom& rhs) {
    if (&rhs != this) {
        this->Clear();
        this->copy(rhs);
    }
}

//------------------------------------------------------------------------------
inline void
StringAtom::operator=(StringAtom&& rhs) {
    if (&rhs != this) {
        this->Clear();
        this->copy(rhs);
        rhs.data = nullptr;
    }
}

//------------------------------------------------------------------------------
inline void
StringAtom::operator=(const char* rhs) {
    this->Clear();
    this->setupFromCString(rhs);
}

//------------------------------------------------------------------------------
inline void
StringAtom::operator=(const uchar* rhs) {
    this->Clear();
    this->setupFromCString((const char*)rhs);
}

//------------------------------------------------------------------------------
inline bool
StringAtom::operator!=(const StringAtom& rhs) const {
    return !operator==(rhs);
}

//------------------------------------------------------------------------------
inline bool
StringAtom::operator<(const StringAtom& rhs) const {
    if (rhs.data && this->data) {
        // it is forbidden to compare string from different threads!
        o_assert(this->data->table == rhs.data->table);
    }
    return this->data < rhs.data;
}

//------------------------------------------------------------------------------
inline bool
StringAtom::operator!=(const char* rhs) const {
    return !operator==(rhs);
}

//------------------------------------------------------------------------------
inline bool
StringAtom::operator==(const uchar* rhs) const {
    return operator==((const char*) rhs);
}

//------------------------------------------------------------------------------
inline bool
StringAtom::IsValid() const {
    return (0 != this->data);
}

//------------------------------------------------------------------------------
inline bool
StringAtom::Empty() const {
    return (0 == this->data);
}

//------------------------------------------------------------------------------
inline int32
StringAtom::Length() const {
    if (nullptr != this->data) {
        return this->data->length;
    }
    else {
        return 0;
    }
}

//------------------------------------------------------------------------------
inline const char*
StringAtom::AsCStr() const {
    if (nullptr != this->data) {
        return this->data->str;
    }
    else {
        return emptyString;
    }
}

} // namespace Oryol
