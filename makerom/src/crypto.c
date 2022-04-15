#include "lib.h"
#include "crypto.h"

#include <polarssl/rsa.h>

#include <mbedtls/aes.h>
#include <mbedtls/rsa.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>

const u8 RSA_PUB_EXP[0x3] = {0x01,0x00,0x01};
const int HASH_MAX_LEN = 0x20;

bool VerifySha256(void *data, u64 size, u8 hash[32])
{
	u8 calchash[32];
	ShaCalc(data, size, calchash, CTR_SHA_256);
	return memcmp(hash,calchash,32) == 0;
}

void ShaCalc(void *data, u64 size, u8 *hash, int mode)
{
	switch(mode){
		case(CTR_SHA_1): mbedtls_sha1((u8*)data, size, hash); break;
		case(CTR_SHA_256): mbedtls_sha256((u8*)data, size, hash, 0); break;
	}
}

void SetAesCtrOffset(u8 *ctr, u64 offset)
{
	u64_to_u8(ctr+8,u8_to_u64(ctr+8,BE)|align(offset,16)/16,BE);
}

void AesCtrCrypt(u8 *key, u8 *ctr, u8 *input, u8 *output, u64 length, u64 offset)
{
	u8 stream[16];
	mbedtls_aes_context aes;
	size_t nc_off = 0;
	
	mbedtls_aes_init(&aes);
	mbedtls_aes_setkey_enc(&aes, key, 128);
	SetAesCtrOffset(ctr,offset);
	
	mbedtls_aes_crypt_ctr(&aes, length, &nc_off, ctr, stream, input, output);
	
	return;
}

void AesCbcCrypt(u8 *key, u8 *iv, u8 *input, u8 *output, u64 length, u8 mode)
{
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	
	switch(mode){
		case(ENC): 
			mbedtls_aes_setkey_enc(&aes, key, 128);
			mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, length, iv, input, output);
			return;
		case(DEC):
			mbedtls_aes_setkey_dec(&aes, key, 128);
			mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, length, iv, input, output); 
			return;
		default:
			return;
	}
}

bool RsaKeyInit(rsa_context* ctx, u8 *modulus, u8 *private_exp, u8 *exponent, u8 rsa_type)
{
	// Sanity Check
	if(!ctx)
		return false;
	
	rsa_init(ctx, RSA_PKCS_V15, 0);
	
	u16 n_size = 0;
	u16 d_size = 0;
	u16 e_size = 0;
	
	switch(rsa_type){
		case RSA_2048:
			ctx->len = 0x100;
			n_size = 0x100;
			d_size = 0x100;
			e_size = 3;
			break;
		case RSA_4096:
			ctx->len = 0x200;
			n_size = 0x200;
			d_size = 0x200;
			e_size = 3;
			break;
		default: return false;
	}
	
	if (modulus && mpi_read_binary(&ctx->N, modulus, n_size))
		goto clean;
	if (exponent && mpi_read_binary(&ctx->E, exponent, e_size))
		goto clean;
	if (private_exp && mpi_read_binary(&ctx->D, private_exp, d_size))
		goto clean;
	

	return true;
clean:
	rsa_free(ctx);
	return false;
}

u8 GetRsaType(u32 sig_type)
{
	switch(sig_type){
		case RSA_4096_SHA1:
		case RSA_4096_SHA256:
			return RSA_4096;
		case RSA_2048_SHA1:
		case RSA_2048_SHA256:
			return RSA_2048;			
	}
	return INVALID_SIG_TYPE;
}

u32 GetSigHashType(u32 sig_type)
{
	switch(sig_type){
		case RSA_4096_SHA1:
		case RSA_2048_SHA1:
		case ECC_SHA1:
			return CTR_SHA_1;
		case RSA_4096_SHA256:
		case RSA_2048_SHA256:
		case ECC_SHA256:
			return CTR_SHA_256;
	}
	return 0;
}

int GetRsaHashType(u32 sig_type)
{
	switch(sig_type){
		case RSA_4096_SHA1:
		case RSA_2048_SHA1:
			return SIG_RSA_SHA1;
		case RSA_4096_SHA256:
		case RSA_2048_SHA256:
			return SIG_RSA_SHA256;
	}
	return 0;
}

u32 GetSigHashLen(u32 sig_type)
{
	switch(sig_type){
		case RSA_4096_SHA1:
		case RSA_2048_SHA1:
		case ECC_SHA1:
			return SHA_1_LEN;
		case RSA_4096_SHA256:
		case RSA_2048_SHA256:
		case ECC_SHA256:
			return SHA_256_LEN;
	}
	return 0;
}

bool CalcHashForSign(void *data, u64 len, u8 *hash, u32 sig_type)
{
	if(GetSigHashType(sig_type) == 0)
		return false;

	ShaCalc(data, len, hash, GetSigHashType(sig_type));
	
	return true;
}

int RsaSignVerify(void *data, u64 len, u8 *sign, u8 *mod, u8 *priv_exp, u32 sig_type, u8 rsa_mode)
{
	int rsa_result = 0;
	rsa_context ctx;
	u8 hash[HASH_MAX_LEN];
		
	if(!RsaKeyInit(&ctx, mod, priv_exp, (u8*)RSA_PUB_EXP, GetRsaType(sig_type)))
		return -1;
		
	if(!CalcHashForSign(data, len, hash, sig_type))
		return -1;		

	if(rsa_mode == CTR_RSA_VERIFY)
		rsa_result = rsa_pkcs1_verify(&ctx, RSA_PUBLIC, GetRsaHashType(sig_type), 0, hash, sign);
	else // CTR_RSA_SIGN
		rsa_result = rsa_rsassa_pkcs1_v15_sign(&ctx, RSA_PRIVATE, GetRsaHashType(sig_type), 0, hash, sign);
	
	rsa_free(&ctx);
	return rsa_result;
}