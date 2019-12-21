#ifndef __CIRCULAR_BUFFER__
#define __CIRCULAR_BUFFER__

#include <algorithm>
#include <vector>

#include "misc.h"

/*--------------------------------------------------------------------------------*/
/** Circular buffer are arbitrary type
 *
 * Can be used lock-free by multiple threads concurrent (as long as each thread
 * ONLY reads or writes - e.g. thread A only writes and thread B only writes)
 */
/*--------------------------------------------------------------------------------*/
template<typename T>
class ACircularBuffer
{
public:
    ACircularBuffer(uint_t size = 0) : data(size),
                                       rdptr(0),
                                       wrptr(0) {}
    virtual ~ACircularBuffer() {}

    /*--------------------------------------------------------------------------------*/
    /** Reset BOTH read and write pointers to 0
     *
     * @note for thread safe reset, use either EmptyRead() or EmptyWrite() (depending upon
     * which thread is doing the emptying)
     */
    /*--------------------------------------------------------------------------------*/
    void Reset() {rdptr = wrptr = 0;}

    /*--------------------------------------------------------------------------------*/
    /** 'Empty' circular buffer by altering read pointer
     *
     * @note this is a thread-safe way of emptying the circular buffer (if called by the reading thread)
     */
    /*--------------------------------------------------------------------------------*/
    void EmptyRead() {rdptr = wrptr;}

    /*--------------------------------------------------------------------------------*/
    /** 'Empty' circular buffer by altering write pointer
     *
     * @note this is a thread-safe way of emptying the circular buffer (if called by the writing thread)
     */
    /*--------------------------------------------------------------------------------*/
    void EmptyWrite() {wrptr = rdptr;}

    /*--------------------------------------------------------------------------------*/
    /** Resize circular buffer
     *
     * @note this is NOT thread safe for several reasons!
     */
    /*--------------------------------------------------------------------------------*/
    void Resize(uint_t n) {data.resize(n); rdptr = wrptr = 0;}

    /*--------------------------------------------------------------------------------*/
    /** Return the number of slots that can be written to
     */
    /*--------------------------------------------------------------------------------*/
    uint_t GetWriteSpaceAvailable() const {return ((rdptr + 2 * data.size() - 1 - wrptr) % data.size());}

    /*--------------------------------------------------------------------------------*/
    /** Return the number of slots that can be read from
     */
    /*--------------------------------------------------------------------------------*/
    uint_t GetReadItemsAvailable()  const {return ((wrptr + data.size() - rdptr) % data.size());}

    /*--------------------------------------------------------------------------------*/
    /** Advance the write pointer by the specified amount
     *
     * @note the advance is limited to what is allowable
     */
    /*--------------------------------------------------------------------------------*/
    uint_t AdvanceWritePointer(uint_t n = 1) const {n = std::min(n, GetWriteSpaceAvailable()); wrptr = (wrptr + n) % data.size(); return n;}

    /*--------------------------------------------------------------------------------*/
    /** Advance the read pointer by the specified amount
     *
     * @note the advance is limited to what is allowable
     */
    /*--------------------------------------------------------------------------------*/
    uint_t AdvanceReadPointer(uint_t n = 1)  const {n = std::min(n, GetReadItemsAvailable());  rdptr = (rdptr + n) % data.size(); return n;}

    /*--------------------------------------------------------------------------------*/
    /** Return the item at the specified offset from the current read pointer
     *
     * @note this DOES NOT CHECK to see if that item is safe to read, use GetReadItemsAvailable() first!
     */
    /*--------------------------------------------------------------------------------*/
    const T& GetReadItem(uint_t offset = 0)  const {return data[(rdptr + offset) % data.size()];}

    /*--------------------------------------------------------------------------------*/
    /** Return access to the item at the specified offset from the current write pointer
     *
     * @note this DOES NOT CHECK to see if that item is safe to writer, use GetWriteSpaceAvailable() first!
     */
    /*--------------------------------------------------------------------------------*/
    T&       GetWriteItem(uint_t offset = 0) const {return data[(wrptr + offset) % data.size()];}

    /*--------------------------------------------------------------------------------*/
    /** write to the item at the current write pointer and advance the write pointer by 1
     *
     * @return true if item has bee written safely
     *
     * @note this prevents unsafe writes
     */
    /*--------------------------------------------------------------------------------*/
    bool WriteItemAndInc(const T& val) {
        bool success = (WriteSpaceAvailable() > 0);
        if (success) {
            data[wrptr] = val;
            AdvanceWritePointer();
        }
        return success;
    }

    /*--------------------------------------------------------------------------------*/
    /** Read the item at the current read pointer and advance the read pointer by 1
     *
     * @return true if item has been read safely
     *
     * @note this prevents unsafe reads
     */
    /*--------------------------------------------------------------------------------*/
    bool ReadItemAndInc(T& res) {
        bool success = (ReadItemsAvailable() > 0);
        if (success) {
            res = data[rdptr];
            AdavnceReadPointer();
        }
        return success;
    }

protected:
    std::vector<T> data;
    uint_t         rdptr;
    uint_t         wrptr;
};

#endif
