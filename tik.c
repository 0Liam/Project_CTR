#include "lib.h"
#include "cia.h"
#include "tik.h"

// Private Prototypes
int SetupTicketBuffer(buffer_struct *tik);
int SetupTicketHeader(tik_hdr *hdr, cia_settings *ciaset);
int SignTicketHeader(tik_hdr *hdr, tik_signature *sig, keys_struct *keys);
void SetLimits(tik_hdr *hdr, cia_settings *ciaset);
void SetContentIndexData(tik_hdr *hdr, cia_settings *ciaset);


int BuildTicket(cia_settings *ciaset)
{
	int result = 0;
	result = SetupTicketBuffer(&ciaset->ciaSections.tik);
	if(result) return result;
	
	// Setting Ticket Struct Ptrs
	tik_signature *sig = (tik_signature*)ciaset->ciaSections.tik.buffer;
	tik_hdr *hdr = (tik_hdr*)(ciaset->ciaSections.tik.buffer+sizeof(tik_signature));

	result = SetupTicketHeader(hdr,ciaset);
	if(result) return result;
	result = SignTicketHeader(hdr,sig,ciaset->keys);
	return 0;
}

int SetupTicketBuffer(buffer_struct *tik)
{
	tik->size = sizeof(tik_signature) + sizeof(tik_hdr);
	tik->buffer = calloc(1,tik->size);
	if(!tik->buffer) { 
		fprintf(stderr,"[TIK ERROR] Not enough memory\n"); 
		return MEM_ERROR; 
	}
	return 0;
}

int SetupTicketHeader(tik_hdr *hdr, cia_settings *ciaset)
{
	memset(hdr,0,sizeof(tik_hdr));

	memcpy(hdr->issuer,ciaset->tik.issuer,0x40);
	hdr->formatVersion = ciaset->tik.formatVersion;
	hdr->caCrlVersion = ciaset->cert.caCrlVersion;
	hdr->signerCrlVersion = ciaset->cert.signerCrlVersion;
	if(ciaset->content.encryptCia)
		CryptTitleKey(hdr->encryptedTitleKey, ciaset->common.titleKey,ciaset->common.titleId,ciaset->keys,ENC);
	memcpy(hdr->ticketId,ciaset->tik.ticketId,8);
	memcpy(hdr->deviceId,ciaset->tik.deviceId,8);
	memcpy(hdr->titleId,ciaset->common.titleId,8);
	u16_to_u8(hdr->ticketVersion,ciaset->tik.version,BE);
	hdr->licenceType = ciaset->tik.licenceType;
	hdr->keyId = ciaset->keys->aes.currentCommonKey;
	memcpy(hdr->eshopAccId,ciaset->tik.eshopAccId,4);
	hdr->audit = ciaset->tik.audit;
	SetLimits(hdr,ciaset);
	SetContentIndexData(hdr,ciaset);
	return 0;
}

int SignTicketHeader(tik_hdr *hdr, tik_signature *sig, keys_struct *keys)
{
	memset(sig,0,sizeof(tik_signature));
	u32_to_u8(sig->sigType,RSA_2048_SHA256,BE);
	return ctr_sig((u8*)hdr,sizeof(tik_hdr),sig->data,keys->rsa.xsPub,keys->rsa.xsPvt,RSA_2048_SHA256,CTR_RSA_SIGN);
}

int CryptTitleKey(u8 *EncTitleKey, u8 *DecTitleKey, u8 *TitleID, keys_struct *keys, u8 mode)
{
	//Generating IV
	u8 iv[16];
	memset(&iv,0x0,16);
	memcpy(iv,TitleID,0x8);
	
	//Setting up Aes Context
	ctr_aes_context ctx;
	memset(&ctx,0x0,sizeof(ctr_aes_context));
	
	//Crypting TitleKey
	ctr_init_aes_cbc(&ctx,keys->aes.commonKey[keys->aes.currentCommonKey],iv,mode);
	if(mode == ENC) ctr_aes_cbc(&ctx,DecTitleKey,EncTitleKey,0x10,ENC);
	else ctr_aes_cbc(&ctx,EncTitleKey,DecTitleKey,0x10,DEC);

	// Return
	return 0;
}

void SetLimits(tik_hdr *hdr, cia_settings *ciaset)
{
	memset(hdr->limits,0,0x40);
}

void SetContentIndexData(tik_hdr *hdr, cia_settings *ciaset)
{
	memset(hdr->contentIndex,0,0xAC);
	memcpy(hdr->contentIndex,default_contentIndex,0x30);
}
