
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blowfish.h"
#include "blowfish_imp.h"

/*--------------------------------------------------------------------------------*/
/** Construct Blowfish object
 *
 * @param key pointer to binary key or NULL for no key
 * @param keyLen number of bytes in key or 0
 *
 */
/*--------------------------------------------------------------------------------*/
ABlowfish::ABlowfish(const uint8_t *key, uint_t keyLen) : context(NULL)
{
    SetKey(key, keyLen);
}

ABlowfish::~ABlowfish()
{
    DeleteContext();
}

/*--------------------------------------------------------------------------------*/
/** Get/create context (private) data
 *
 * @note data is zero'd after creation
 */
/*--------------------------------------------------------------------------------*/
void *ABlowfish::GetContext()
{
    if (!context) {
        if ((context = new BLOWFISH_CTX) != NULL) {
            memset(context, 0, sizeof(BLOWFISH_CTX));
        }
    }

    return context;
}

/*--------------------------------------------------------------------------------*/
/** Delete context (private) data
 *
 * @note data is zero'd BEFORE deletion
 */
/*--------------------------------------------------------------------------------*/
void ABlowfish::DeleteContext()
{
    if (context) {
        BLOWFISH_CTX *ctx = (BLOWFISH_CTX *)GetContext();
        memset(ctx, 0, sizeof(*ctx));
        delete ctx;

        context = NULL;
    }
}

/*--------------------------------------------------------------------------------*/
/** Set Blowfish key
 *
 * @param key pointer to binary key or NULL for no key
 * @param keyLen number of bytes in key or 0
 *
 * @note can be used to clear current key if passed NULL / 0
 */
/*--------------------------------------------------------------------------------*/
void ABlowfish::SetKey(const uint8_t *key, uint_t keyLen)
{
    if (key && keyLen) {
        // valid key -> create and initialise context
        BLOWFISH_CTX *ctx = (BLOWFISH_CTX *)GetContext();

        // zero context for safety
        memset(ctx, 0, sizeof(*ctx));

        // initialise Blowfish tables with key
        Blowfish_Init(ctx, key, keyLen);
    }
    // no key -> delete existing context
    else DeleteContext();
}

/*--------------------------------------------------------------------------------*/
/** Encrypt a single block of data
 *
 * @param src source
 * @param dst destination
 * @param iv1 initialisation vector 1
 * @param iv2 initialisation vector 2
 *
 * @note this function ALWAYS encrypts BLOCKSIZE bytes!
 * @note iv1 and iv2 are updated by this function. REPEATEDLY set these to 0 to run in ECB mode
 *
 */
/*--------------------------------------------------------------------------------*/
void ABlowfish::Encrypt(const uint8_t *src, uint8_t *dst, uint32_t& iv1, uint32_t& iv2)
{
    BLOWFISH_CTX *ctx = (BLOWFISH_CTX *)GetContext();
    uint32_t l, r;

    // copy bytes into 2 32-bit variables
    memcpy(&l, src, sizeof(l)); src += sizeof(l);
    memcpy(&r, src, sizeof(r));

    // for little endian machines, swap bytes to make all data big-endian
    if (!MachineIsBigEndian()) {
        l = SwapBytes(l);
        r = SwapBytes(r);
    }

    // XOR in initialisation vectors (for CBC mode)
    l ^= iv1;
    r ^= iv2;

    // encrypt data
    Blowfish_Encrypt(ctx, &l, &r);

    // return new initialisation vectors (for CBC mode)
    iv1 = l;
    iv2 = r;

    // for little endian machines, swap bytes back to make all data little-endian
    if (!MachineIsBigEndian()) {
        l = SwapBytes(l);
        r = SwapBytes(r);
    }

    // copy bytes to destination
    memcpy(dst, &l, sizeof(l)); dst += sizeof(l);
    memcpy(dst, &r, sizeof(r));

    l = r = 0;  // clear local variables to prevent security leaks
}

/*--------------------------------------------------------------------------------*/
/** Decrypt a single block of data
 *
 * @param src source
 * @param dst destination
 * @param iv1 initialisation vector 1
 * @param iv2 initialisation vector 2
 *
 * @note this function ALWAYS decrypts BLOCKSIZE bytes!
 * @note iv1 and iv2 are updated by this function. REPEATEDLY set these to 0 to run in ECB mode
 *
 */
/*--------------------------------------------------------------------------------*/
void ABlowfish::Decrypt(const uint8_t *src, uint8_t *dst, uint32_t& iv1, uint32_t& iv2)
{
    BLOWFISH_CTX *ctx = (BLOWFISH_CTX *)GetContext();
    uint32_t l, r, iv1_1 = iv1, iv2_1 = iv2;

    // copy bytes into 2 32-bit variables
    memcpy(&l, src, sizeof(l));
    memcpy(&r, src + sizeof(l), sizeof(r));

    // for little endian machines, swap bytes to make all data big-endian
    if (!MachineIsBigEndian()) {
        l = SwapBytes(l);
        r = SwapBytes(r);
    }

    // return new initialisation vectors (for CBC mode)
    iv1 = l;
    iv2 = r;

    // decrypt data
    Blowfish_Decrypt(ctx, &l, &r);

    // XOR in initialisation vectors (for CBC mode)
    l ^= iv1_1;
    r ^= iv2_1;

    // zero local variables for security
    iv1_1 = iv2_1 = 0;

    // for little endian machines, swap bytes back to make all data little-endian
    if (!MachineIsBigEndian()) {
        l = SwapBytes(l);
        r = SwapBytes(r);
    }

    // copy bytes to destination
    memcpy(dst, &l, sizeof(l));
    memcpy(dst + sizeof(l), &r, sizeof(r));

    l = r = 0;  // clear local variables to prevent security leaks
}

/*--------------------------------------------------------------------------------*/
/** Encrypt data
 *
 * @param src source array
 * @param dst destination array
 * @param nbytes number of bytes
 * @param cbc true to work in CBC mode otherwise ECB mode will be used
 * @param ivec pointer to TWO uint32_t's to be used as initialisation vectors if cbc = true or NULL
 *
 * @note this function ONLY encrypts WHOLE blocks - any sub-block is left UNENCRYPTED
 * @note ivec can be used to chain multiple blocks together for CBC mode
 * @note ivec can still be NULL if cbc = true in which case an IV of 0 will be used
 *
 */
/*--------------------------------------------------------------------------------*/
void ABlowfish::Encrypt(const uint8_t *src, uint8_t *dst, uint_t nbytes, bool cbc, uint32_t *ivec)
{
    if (context) {
        // setup initialisation vectors
        uint32_t iv1 = (cbc && ivec) ? ivec[0] : 0;
        uint32_t iv2 = (cbc && ivec) ? ivec[1] : 0;

        // encrypt whole blocks
        while (nbytes >= BLOCKSIZE) {
            if (!cbc) {
                // if not running in CBC, zero initialisation vectors)
                iv1 = iv2 = 0;
            }

            Encrypt(src, dst, iv1, iv2);

            src    += BLOCKSIZE;
            dst    += BLOCKSIZE;
            nbytes -= BLOCKSIZE;
        }

        if (cbc && ivec) {
            // if running in CBC mode, update caller's initialisation vectors
            ivec[0] = iv1;
            ivec[1] = iv2;
        }

        // zero local variables
        iv1 = iv2 = 0;
    }
}

/*--------------------------------------------------------------------------------*/
/** Decrypt data
 *
 * @param src source array
 * @param dst destination array
 * @param nbytes number of bytes
 * @param cbc true to work in CBC mode otherwise ECB mode will be used
 * @param ivec pointer to TWO uint32_t's to be used as initialisation vectors if cbc = true or NULL
 *
 * @note this function ONLY decrypts WHOLE blocks - any sub-block is left UNDECRYPTED
 * @note ivec can be used to chain multiple blocks together for CBC mode
 * @note ivec can still be NULL if cbc = true in which case an IV of 0 will be used
 *
 */
/*--------------------------------------------------------------------------------*/
void ABlowfish::Decrypt(const uint8_t *src, uint8_t *dst, uint_t nbytes, bool cbc, uint32_t *ivec)
{
    if (context) {
        // setup initialisation vectors
        uint32_t iv1 = (cbc && ivec) ? ivec[0] : 0;
        uint32_t iv2 = (cbc && ivec) ? ivec[1] : 0;

        // decrypt whole blocks
        while (nbytes >= BLOCKSIZE) {
            if (!cbc) {
                // if not running in CBC, zero initialisation vectors)
                iv1 = iv2 = 0;
            }

            Decrypt(src, dst, iv1, iv2);

            src    += BLOCKSIZE;
            dst    += BLOCKSIZE;
            nbytes -= BLOCKSIZE;
        }

        if (cbc && ivec) {
            // if running in CBC mode, update caller's initialisation vectors
            ivec[0] = iv1;
            ivec[1] = iv2;
        }

        // zero local variables
        iv1 = iv2 = 0;
    }
}
