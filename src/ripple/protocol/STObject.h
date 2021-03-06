//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012, 2013 Ripple Labs Inc.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#ifndef RIPPLE_PROTOCOL_STOBJECT_H_INCLUDED
#define RIPPLE_PROTOCOL_STOBJECT_H_INCLUDED

#include <ripple/basics/CountedObject.h>
#include <ripple/protocol/STAmount.h>
#include <ripple/protocol/STPathSet.h>
#include <ripple/protocol/STVector256.h>
#include <ripple/protocol/SOTemplate.h>
#include <boost/ptr_container/ptr_vector.hpp>

namespace ripple {

class STArray;

class STObject
    : public STBase
    , public CountedObject <STObject>
{
public:
    static char const* getCountedObjectName () { return "STObject"; }

    STObject () : mType (nullptr)
    {
        ;
    }

    explicit STObject (SField::ref name)
        : STBase (name), mType (nullptr)
    {
        ;
    }

    STObject (const SOTemplate & type, SField::ref name)
        : STBase (name)
    {
        set (type);
    }

    STObject (
        const SOTemplate & type, SerialIter & sit, SField::ref name)
        : STBase (name)
    {
        set (sit);
        setType (type);
    }

    STObject (SField::ref name, boost::ptr_vector<STBase>& data)
        : STBase (name), mType (nullptr)
    {
        mData.swap (data);
    }

    std::unique_ptr <STObject> oClone () const
    {
        return std::make_unique <STObject> (*this);
    }

    virtual ~STObject () { }

    static std::unique_ptr<STBase>
    deserialize (SerialIter & sit, SField::ref name);

    bool setType (const SOTemplate & type);
    bool isValidForType ();
    bool isFieldAllowed (SField::ref);
    bool isFree () const
    {
        return mType == nullptr;
    }

    void set (const SOTemplate&);
    bool set (SerialIter& u, int depth = 0);

    virtual SerializedTypeID getSType () const override
    {
        return STI_OBJECT;
    }
    virtual bool isEquivalent (const STBase & t) const override;
    virtual bool isDefault () const override
    {
        return mData.empty ();
    }

    virtual void add (Serializer & s) const override
    {
        add (s, true);    // just inner elements
    }

    void add (Serializer & s, bool withSignature) const;

    // VFALCO NOTE does this return an expensive copy of an object with a
    //             dynamic buffer?
    // VFALCO TODO Remove this function and fix the few callers.
    Serializer getSerializer () const
    {
        Serializer s;
        add (s);
        return s;
    }

    virtual std::string getFullText () const override;
    virtual std::string getText () const override;

    // TODO(tom): options should be an enum.
    virtual Json::Value getJson (int options) const override;

    int addObject (const STBase & t)
    {
        mData.push_back (t.duplicate ().release ());
        return mData.size () - 1;
    }
    int giveObject (std::unique_ptr<STBase> t)
    {
        mData.push_back (t.release ());
        return mData.size () - 1;
    }
    int giveObject (STBase * t)
    {
        mData.push_back (t);
        return mData.size () - 1;
    }
    const boost::ptr_vector<STBase>& peekData () const
    {
        return mData;
    }
    boost::ptr_vector<STBase>& peekData ()
    {
        return mData;
    }
    STBase& front ()
    {
        return mData.front ();
    }
    const STBase& front () const
    {
        return mData.front ();
    }
    STBase& back ()
    {
        return mData.back ();
    }
    const STBase& back () const
    {
        return mData.back ();
    }

    int getCount () const
    {
        return mData.size ();
    }

    bool setFlag (std::uint32_t);
    bool clearFlag (std::uint32_t);
    bool isFlag(std::uint32_t) const;
    std::uint32_t getFlags () const;

    uint256 getHash (std::uint32_t prefix) const;
    uint256 getSigningHash (std::uint32_t prefix) const;

    const STBase& peekAtIndex (int offset) const
    {
        return mData[offset];
    }
    STBase& getIndex (int offset)
    {
        return mData[offset];
    }
    const STBase* peekAtPIndex (int offset) const
    {
        return & (mData[offset]);
    }
    STBase* getPIndex (int offset)
    {
        return & (mData[offset]);
    }

    int getFieldIndex (SField::ref field) const;
    SField::ref getFieldSType (int index) const;

    const STBase& peekAtField (SField::ref field) const;
    STBase& getField (SField::ref field);
    const STBase* peekAtPField (SField::ref field) const;
    STBase* getPField (SField::ref field, bool createOkay = false);

    // these throw if the field type doesn't match, or return default values
    // if the field is optional but not present
    std::string getFieldString (SField::ref field) const;
    unsigned char getFieldU8 (SField::ref field) const;
    std::uint16_t getFieldU16 (SField::ref field) const;
    std::uint32_t getFieldU32 (SField::ref field) const;
    std::uint64_t getFieldU64 (SField::ref field) const;
    uint128 getFieldH128 (SField::ref field) const;

    uint160 getFieldH160 (SField::ref field) const;
    uint256 getFieldH256 (SField::ref field) const;
    RippleAddress getFieldAccount (SField::ref field) const;
    Account getFieldAccount160 (SField::ref field) const;

    Blob getFieldVL (SField::ref field) const;
    STAmount const& getFieldAmount (SField::ref field) const;
    STPathSet const& getFieldPathSet (SField::ref field) const;
    const STVector256& getFieldV256 (SField::ref field) const;
    const STArray& getFieldArray (SField::ref field) const;

    /** Set a field.
        if the field already exists, it is replaced.
    */
    void
    set (std::unique_ptr<STBase> v);

    void setFieldU8 (SField::ref field, unsigned char);
    void setFieldU16 (SField::ref field, std::uint16_t);
    void setFieldU32 (SField::ref field, std::uint32_t);
    void setFieldU64 (SField::ref field, std::uint64_t);
    void setFieldH128 (SField::ref field, uint128 const&);
    void setFieldH256 (SField::ref field, uint256 const& );
    void setFieldVL (SField::ref field, Blob const&);
    void setFieldAccount (SField::ref field, Account const&);
    void setFieldAccount (SField::ref field, RippleAddress const& addr)
    {
        setFieldAccount (field, addr.getAccountID ());
    }
    void setFieldAmount (SField::ref field, STAmount const&);
    void setFieldPathSet (SField::ref field, STPathSet const&);
    void setFieldV256 (SField::ref field, STVector256 const& v);
    void setFieldArray (SField::ref field, STArray const& v);

    template <class Tag>
    void setFieldH160 (SField::ref field, base_uint<160, Tag> const& v)
    {
        STBase* rf = getPField (field, true);

        if (!rf)
            throw std::runtime_error ("Field not found");

        if (rf->getSType () == STI_NOTPRESENT)
            rf = makeFieldPresent (field);

        using Bits = STBitString<160>;
        if (auto cf = dynamic_cast<Bits*> (rf))
            cf->setValue (v);
        else
            throw std::runtime_error ("Wrong field type");
    }

    STObject& peekFieldObject (SField::ref field);

    bool isFieldPresent (SField::ref field) const;
    STBase* makeFieldPresent (SField::ref field);
    void makeFieldAbsent (SField::ref field);
    bool delField (SField::ref field);
    void delField (int index);

    static std::unique_ptr <STBase>
    makeDefaultObject (SerializedTypeID id, SField::ref name);

    // VFALCO TODO remove the 'depth' parameter
    static std::unique_ptr<STBase> makeDeserializedObject (
        SerializedTypeID id,
        SField::ref name,
        SerialIter&,
        int depth);

    static std::unique_ptr<STBase>
    makeNonPresentObject (SField::ref name)
    {
        return makeDefaultObject (STI_NOTPRESENT, name);
    }

    static std::unique_ptr<STBase> makeDefaultObject (SField::ref name)
    {
        return makeDefaultObject (name.fieldType, name);
    }

    // field iterator stuff
    typedef boost::ptr_vector<STBase>::iterator iterator;
    typedef boost::ptr_vector<STBase>::const_iterator const_iterator;
    iterator begin ()
    {
        return mData.begin ();
    }
    iterator end ()
    {
        return mData.end ();
    }
    const_iterator begin () const
    {
        return mData.begin ();
    }
    const_iterator end () const
    {
        return mData.end ();
    }
    bool empty () const
    {
        return mData.empty ();
    }

    bool hasMatchingEntry (const STBase&);

    bool operator== (const STObject & o) const;
    bool operator!= (const STObject & o) const
    {
        return ! (*this == o);
    }

    std::unique_ptr<STBase>
    duplicate () const override
    {
        return std::make_unique<STObject>(*this);
    }

private:
    // Implementation for getting (most) fields that return by value.
    //
    // The remove_cv and remove_reference are necessitated by the STBitString
    // types.  Their getValue returns by const ref.  We return those types
    // by value.
    template <typename T, typename V =
        typename std::remove_cv < typename std::remove_reference <
            decltype (std::declval <T> ().getValue ())>::type >::type >
    V getFieldByValue (SField::ref field) const
    {
        const STBase* rf = peekAtPField (field);

        if (!rf)
            throw std::runtime_error ("Field not found");

        SerializedTypeID id = rf->getSType ();

        if (id == STI_NOTPRESENT)
            return V (); // optional field not present

        const T* cf = dynamic_cast<const T*> (rf);

        if (!cf)
            throw std::runtime_error ("Wrong field type");

        return cf->getValue ();
    }

    // Implementations for getting (most) fields that return by const reference.
    //
    // If an absent optional field is deserialized we don't have anything
    // obvious to return.  So we insist on having the call provide an
    // 'empty' value we return in that circumstance.
    template <typename T, typename V>
    V const& getFieldByConstRef (SField::ref field, V const& empty) const
    {
        const STBase* rf = peekAtPField (field);

        if (!rf)
            throw std::runtime_error ("Field not found");

        SerializedTypeID id = rf->getSType ();

        if (id == STI_NOTPRESENT)
            return empty; // optional field not present

        const T* cf = dynamic_cast<const T*> (rf);

        if (!cf)
            throw std::runtime_error ("Wrong field type");

        return *cf;
    }

    // Implementation for setting most fields with a setValue() method.
    template <typename T, typename V>
    void setFieldUsingSetValue (SField::ref field, V value)
    {
        static_assert(!std::is_lvalue_reference<V>::value, "");

        STBase* rf = getPField (field, true);

        if (!rf)
            throw std::runtime_error ("Field not found");

        if (rf->getSType () == STI_NOTPRESENT)
            rf = makeFieldPresent (field);

        T* cf = dynamic_cast<T*> (rf);

        if (!cf)
            throw std::runtime_error ("Wrong field type");

        cf->setValue (std::move (value));
    }

    // Implementation for setting fields using assignment
    template <typename T>
    void setFieldUsingAssignment (SField::ref field, T const& value)
    {
        STBase* rf = getPField (field, true);

        if (!rf)
            throw std::runtime_error ("Field not found");

        if (rf->getSType () == STI_NOTPRESENT)
            rf = makeFieldPresent (field);

        T* cf = dynamic_cast<T*> (rf);

        if (!cf)
            throw std::runtime_error ("Wrong field type");

        (*cf) = value;
    }

private:
    boost::ptr_vector<STBase> mData;
    const SOTemplate* mType;
};

} // ripple

#endif
