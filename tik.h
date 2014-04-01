#ifndef _TIK_H_
#define _TIK_H_

static const unsigned char default_contentIndex[0x30] =
{
	0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0xAC, 
	0x00, 0x00, 0x00, 0x14, 0x00, 0x01, 0x00, 0x14, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x84, 
	0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};

typedef enum
{
	lic_Permanent = 0,
	lic_Demo = 1,
	lic_Trial = 2,
	lic_Rental = 3,
	lic_Subscription = 4,
	lic_Service = 5,
	lic_Mask = 15
} tik_license_type;

typedef enum
{
	right_Permanent = 1,
	right_Subscription = 2,
	right_Content = 3,
	right_ContentConsumption = 4,
	right_AccessTitle = 5
} tik_item_rights;

typedef struct
{
	u8 sigType[4];
	u8 data[0x100];
	u8 padding[0x3C];
} tik_signature;

typedef struct
{
	u8 issuer[0x40];
	u8 eccPubKey[0x3c];
	u8 formatVersion;
	u8 caCrlVersion;
	u8 signerCrlVersion;
	u8 encryptedTitleKey[0x10];
	u8 padding0;
	u8 ticketId[8];
	u8 deviceId[4];
	u8 titleId[8];
	u8 padding1[2];
	u8 ticketVersion[2];
	u8 padding2[8];
	u8 licenceType;
	u8 keyId;
	u8 padding3[0x2A];
	u8 eshopAccId[4];
	u8 padding4;
	u8 audit;
	u8 padding5[0x42];
	u8 limits[0x40];
	u8 contentIndex[0xAC];
} tik_hdr;

#endif

// Prototypes
int BuildTicket(cia_settings *ciaset);
int CryptTitleKey(u8 *EncTitleKey, u8 *DecTitleKey, u8 *TitleID, keys_struct *keys, u8 mode);