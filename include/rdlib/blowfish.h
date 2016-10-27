#ifndef __BLOWFISH__
#define __BLOWFISH__

#include "misc.h"

class ABlowfish {
public:
	/*--------------------------------------------------------------------------------*/
	/** Construct Blowfish object
	 *
	 * @param key pointer to binary key or NULL for no key
	 * @param keyLen number of bytes in key or 0
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	ABlowfish(const uint8_t *key = NULL, uint_t keyLen = 0);
	~ABlowfish();

	enum {
		BLOCKSIZE = 2 * sizeof(uint32_t)
	};

	/*--------------------------------------------------------------------------------*/
	/** Set Blowfish key
	 *
	 * @param key pointer to binary key or NULL for no key
	 * @param keyLen number of bytes in key or 0
	 *
	 * @note can be used to clear current key if passed NULL / 0
	 */
	/*--------------------------------------------------------------------------------*/
	void SetKey(const uint8_t *key, uint_t keyLen);

	/*--------------------------------------------------------------------------------*/
	/** Return whether object is properly initialised and has had key set
	 */
	/*--------------------------------------------------------------------------------*/
	bool IsValid() const {return (context != NULL);}

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
	void Encrypt(const uint8_t *src, uint8_t *dst, uint_t nbytes, bool cbc = true, uint32_t *ivec = NULL);

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
	void Decrypt(const uint8_t *src, uint8_t *dst, uint_t nbytes, bool cbc = true, uint32_t *ivec = NULL);

protected:
	/*--------------------------------------------------------------------------------*/
	/** Get/create context (private) data
	 */
	/*--------------------------------------------------------------------------------*/
	void *GetContext();

	/*--------------------------------------------------------------------------------*/
	/** Delete context (private) data
	 */
	/*--------------------------------------------------------------------------------*/
	void DeleteContext();

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
	void Encrypt(const uint8_t *src, uint8_t *dst, uint32_t& iv1, uint32_t& iv2);

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
	void Decrypt(const uint8_t *src, uint8_t *dst, uint32_t& iv1, uint32_t& iv2);

private:
	void *context;
};

#endif
