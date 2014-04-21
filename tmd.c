#include "lib.h"
#include "cia.h"
#include "tmd.h"

// Private Prototypes
int SetupTMDBuffer(buffer_struct *tik);
int SetupTMDHeader(tmd_hdr *hdr, tmd_content_info_record *info_record, cia_settings *ciaset);
int SignTMDHeader(tmd_hdr *hdr, tmd_signature *sig, keys_struct *keys);
int SetupTMDInfoRecord(tmd_content_info_record *info_record, u8 *content_record, u16 ContentCount);
int SetupTMDContentRecord(u8 *content_record, cia_settings *ciaset);

u32 PredictTMDSize(u16 ContentCount)
{
	return sizeof(tmd_signature) + sizeof(tmd_hdr) + sizeof(tmd_content_info_record)*64 + sizeof(tmd_content_chunk)*ContentCount;
}

int BuildTMD(cia_settings *ciaset)
{
	int result = 0;
	ciaset->ciaSections.tmd.size = PredictTMDSize(ciaset->content.contentCount);
	result = SetupTMDBuffer(&ciaset->ciaSections.tmd);
	if(result) return result;

	// Setting TMD Struct Ptrs
	tmd_signature *sig = (tmd_signature*)ciaset->ciaSections.tmd.buffer;
	tmd_hdr *hdr = (tmd_hdr*)(ciaset->ciaSections.tmd.buffer+sizeof(tmd_signature));
	tmd_content_info_record *info_record = (tmd_content_info_record*)(ciaset->ciaSections.tmd.buffer+sizeof(tmd_signature)+sizeof(tmd_hdr));
	u8 *content_record = (u8*)(ciaset->ciaSections.tmd.buffer+sizeof(tmd_signature)+sizeof(tmd_hdr)+sizeof(tmd_content_info_record)*64);


	SetupTMDContentRecord(content_record,ciaset);
	SetupTMDInfoRecord(info_record,content_record,ciaset->content.contentCount);
	result = SetupTMDHeader(hdr,info_record,ciaset);
	if(result) return result;
	result = SignTMDHeader(hdr,sig,ciaset->keys);
	return 0;
}

int SetupTMDBuffer(buffer_struct *tmd)
{
	// Predict TMD Size
	tmd->buffer = calloc(1,tmd->size); // tmd->size is already set before
	if(!tmd->buffer) { 
		fprintf(stderr,"[ERROR] Not enough memory\n"); 
		return MEM_ERROR; 
	}
	return 0;
}

int SetupTMDHeader(tmd_hdr *hdr, tmd_content_info_record *info_record, cia_settings *ciaset)
{
	memset(hdr,0,sizeof(tmd_hdr));

	memcpy(hdr->issuer,ciaset->tmd.issuer,0x40);
	hdr->formatVersion = ciaset->tmd.formatVersion;
	hdr->caCrlVersion = ciaset->cert.caCrlVersion;
	hdr->signerCrlVersion = ciaset->cert.signerCrlVersion;
	memcpy(hdr->titleID,ciaset->common.titleId,8);
	memcpy(hdr->titleType,ciaset->tmd.titleType,4);
	memcpy(hdr->savedataSize,ciaset->tmd.savedataSize,4);
	memcpy(hdr->privSavedataSize,ciaset->tmd.privSavedataSize,4);
	hdr->twlFlag = ciaset->tmd.twlFlag;
	u16_to_u8(hdr->titleVersion,ciaset->tmd.version,BE);
	u16_to_u8(hdr->contentCount,ciaset->content.contentCount,BE);
	ctr_sha(info_record,sizeof(tmd_content_info_record)*64,hdr->infoRecordHash,CTR_SHA_256);
	return 0;
}

int SignTMDHeader(tmd_hdr *hdr, tmd_signature *sig, keys_struct *keys)
{
	memset(sig,0,sizeof(tmd_signature));
	u32_to_u8(sig->sigType,RSA_2048_SHA256,BE);
	return ctr_sig((u8*)hdr,sizeof(tmd_hdr),sig->data,keys->rsa.cpPub,keys->rsa.cpPvt,RSA_2048_SHA256,CTR_RSA_SIGN);
}

int SetupTMDInfoRecord(tmd_content_info_record *info_record, u8 *content_record, u16 ContentCount)
{
	memset(info_record,0x0,sizeof(tmd_content_info_record)*0x40);
	u16_to_u8(info_record->contentIndexOffset,0x0,BE);
	u16_to_u8(info_record->contentCommandCount,ContentCount,BE);
	ctr_sha(content_record,sizeof(tmd_content_chunk)*ContentCount,info_record->contentChunkHash,CTR_SHA_256);
	return 0;
}

int SetupTMDContentRecord(u8 *content_record, cia_settings *ciaset)
{
	for(int i = 0; i < ciaset->content.contentCount; i++){
		tmd_content_chunk *ptr = (tmd_content_chunk*)(content_record+sizeof(tmd_content_chunk)*i);
		u32_to_u8(ptr->contentID,ciaset->content.contentId[i],BE);
		u16_to_u8(ptr->contentIndex,ciaset->content.contentIndex[i],BE);
		u16_to_u8(ptr->contentFlags,ciaset->content.contentFlags[i],BE);
		u64_to_u8(ptr->contentSize,ciaset->content.contentSize[i],BE);
		memcpy(ptr->contentHash,ciaset->content.contentHash[i],0x20);
	}
	return 0;
}