/*++

Copyright 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    Tcpa11.h

Abstract:

  TCPA Specification data structures

  See http://www.trustedpc.org for the latest architecture specification 
  for TCPA.

--*/

#ifndef _EFI_TCPA_H_
#define _EFI_TCPA_H_

//
// From the TCPA 1.1 Specification
//
typedef UINT32  TCPA_PCRINDEX;
typedef UINT32  TCPA_EVENTTYPE;
typedef UINT32  TCPA_DIRINDEX;
typedef UINT32  TCPA_AUTHHANDLE;
typedef UINT32  TSS_HASHHANDLE;
typedef UINT32  TSS_HMACHHANDLE;
typedef UINT32  TCPA_ENCHANDLE;
typedef UINT32  TCPA_KEY_HANDLE;
typedef UINT32  TCPA_RESULT;
typedef UINT32  TCPA_ALGORITHM_ID;
typedef UINT32  TCPA_ENC_SCHEME;
typedef UINT32  TCPA_SIG_SCHEME;
typedef UINT32  TCPA_KEY_USAGE;
typedef UINT32  TCPA_COMMAND_CODE;

typedef BOOLEAN TCPA_AUTH_DATA_USAGE;
typedef BOOLEAN TCPA_SECRET;

typedef UINT8   TCPA_DIRVALUE;
typedef UINT8   TCPA_PCRVALUE;
typedef UINT32  TCPA_MIGRATE_SCHEME;

#define TPM_TAG_RQU_COMMAND       0x00C1  // A command with no authentication.
#define TPM_TAG_RQU_AUTH1_COMMAND 0x00C2  // An authenticated command with one authentication handle
#define TPM_TAG_RQU_AUTH2_COMMAND 0x00C3  // An authenticated command with two authentication handles
#define TPM_TAG_RSP_COMMAND       0x00C4  // A response from a command with no authentication
#define TPM_TAG_RSP_AUTH1_COMMAND 0x00C5  // An authenticated response with one authentication handle
#define TPM_TAG_RSP_AUTH2_COMMAND 0x00C6  // An authenticated response with two authentication handles


typedef struct {
  UINT8  Major;
  UINT8  Minor;
  UINT8  RevMajor;
  UINT8  RevMinor;
} TCPA_VERSION;

typedef struct {
  UINT8  Nonce[20];
} TCPA_NONCE;

typedef struct  {
  UINT32  KeyLength;
  UINT8   Key[1];
} TCPA_STORE_PUBKEY;

typedef struct {
  TCPA_ALGORITHM_ID   AlgorithmID;
  TCPA_ENC_SCHEME     EncScheme;
  TCPA_SIG_SCHEME     SigScheme;
  UINT32              ParmSize;
  UINT8               Parms[1];
} TCPA_KEY_PARMS;

typedef struct {
  TCPA_KEY_PARMS    AlgorithmParms;
  TCPA_STORE_PUBKEY PubKey;
} TCPA_PUBKEY;


typedef struct {
  UINT32 IsWrappedToPCR : 1;
  UINT32 Redirection    : 1;
  UINT32 Migratable     : 1;
  UINT32 Volatile       : 1;
  UINT32 unused         : 28;
} TCPA_KEY_FLAGS;

typedef struct {
  TCPA_VERSION          Ver;
  TCPA_KEY_USAGE        KeyUsage;
  TCPA_KEY_FLAGS        KeyFlags;
  TCPA_AUTH_DATA_USAGE  AuthDataUsage;
  TCPA_KEY_PARMS        AlgorithmParms;
  UINT32                PCRInfoSize;
  UINT8                 *PCRInfo;
  TCPA_STORE_PUBKEY     PubKey;
  UINT32                EncSize;
  UINT8                 *EncData;
} TCPA_KEY;

#define SHA1_DIGEST_SIZE 20

typedef struct {
 UINT8  Digest[SHA1_DIGEST_SIZE];
} TCPA_DIGEST;


typedef TCPA_DIGEST TCPA_COMPOSITE_HASH;

typedef struct {
  TCPA_PCRINDEX   PCRIndex;
  TCPA_EVENTTYPE  EventType;
  TCPA_DIGEST     PCRValue;
  UINT32          EventSize;
  UINT8           Event[1];
} TCPA_PCR_EVENT;

typedef UINT8 TCPA_AUTHDATA[20];

typedef struct {
  UINT16          Loaded;
  TCPA_KEY_HANDLE Handle[1];
} TCPA_KEY_HANDLE_LIST;

typedef struct {
  UINT8           RevMajor;
  UINT8           RevMinor;
  TCPA_NONCE      TpmProof;
  TCPA_PUBKEY     ManuMaintPub;
  TCPA_KEY        EndorsementKey;
  TCPA_SECRET     OwnerAuth;
  TCPA_KEY        Srk;
  TCPA_DIRVALUE   *Dir;
  TCPA_PCRVALUE   *Pcr;
  UINT8           *RngState;
} TCPA_PERSISTENT_DATA;

typedef struct {
  BOOLEAN  Disable;
  BOOLEAN  Ownership;
  BOOLEAN  Deactivated;
  BOOLEAN  ReadPubek;
  BOOLEAN  DisableOwnerClear;
  BOOLEAN  AllowMaintenance;
} TCPA_PERSISTENT_FLAGS;

typedef struct {
  BOOLEAN  Deactivated;
  BOOLEAN  DisableForceClear;
} TCPA_VOLATILE_FLAGS;

typedef UINT8 TCPA_PAYLOAD_TYPE;

#define TCPA_PT_ASYM    0x01  // The entity is an asymmetric key
#define TCPA_PT_BIND    0x02  // The entity is bound data
#define TCPA_PT_MIGRATE 0x03  // The entity is a migration blob
#define TCPA_PT_MAINT   0x04  // The entity is a maintenance blob
#define TCPA_PT_SEAL    0x05  // The entity is sealed data

typedef UINT16 TCPA_PROTOCOL_ID;

#define TCPA_ALG_RSA  0x00000001  // The RSA algorithm.
#define TCPA_ALG_DES  0x00000002  // The DES algorithm
#define TCPA_ALG_3DES 0X00000003  // The 3DES algorithm
#define TCPA_ALG_SHA  0x00000004  // The SHA1 algorithm
#define TCPA_ALG_HMAC 0x00000005  // The RFC 2104 HMAC algorithm
#define TCPA_ALG_AES  0x00000006  // The AES algorithm

typedef struct { 
  UINT32  KeyLength;
  UINT32  NumPrimes;
  UINT32  ExponentSize;
  UINT8   Exponent[1];
} TCPA_RSA_KEY_PARMS;

typedef struct {
  UINT16  Size;
  UINT8  *Data;
} TCPA_SYMMETRIC_IV;

typedef struct  {
  TCPA_SECRET  NewAuthSecret;
  TCPA_NONCE   N1;
} TCPA_CHANGEAUTH_VALIDATE;


typedef struct {
  TCPA_PUBKEY         MigrationKey;
  TCPA_MIGRATE_SCHEME MigrationScheme;
  TCPA_DIGEST         Digest;
} TCPA_MIGRATIONKEYAUTH;

typedef struct  {
  TCPA_COMMAND_CODE   Ordinal;
  TCPA_RESULT         Returncode;
} TCPA_AUDIT_EVENT;

typedef struct {
  TCPA_DIGEST  CertificateHash; 
  TCPA_DIGEST  EntityDigest;
  BOOLEAN      DigestChecked;
  BOOLEAN      DigestVerified;
  UINT32       IssuerSize;
  UINT8        *Issuer;
} TCPA_EVENT_CERT;


#define EV_CODE_CERT          0
#define EV_CODE_NOCERT        1
#define EV_XML_CONFIG         2
#define EV_NO_ACTION          3
#define EV_SEPARATOR          4
#define EV_ACTION             5
#define EV_PLATFORM_SPECIFIC  6

typedef struct { 
  UINT16 SizeOfSelect;
  UINT8  PcrSelect[1];
} TCPA_PCR_SELECTION;

typedef struct  { 
  TCPA_PCR_SELECTION  Select;
  UINT32              ValueSize;
  TCPA_PCRVALUE       PcrValue[1];
} TCPA_PCR_COMPOSITE;

typedef struct {
  TCPA_PCR_SELECTION  PcrSelection;
  TCPA_COMPOSITE_HASH DigestAtRelease;
  TCPA_COMPOSITE_HASH DigestAtCreation;
} TCPA_PCR_INFO;

typedef struct { 
  TCPA_VERSION  Ver;
  UINT32        SealInfoSize;
  UINT8         *SealInfo;
  UINT32        EncDataSize;
  UINT8*        *EncData;
} TCPA_STORED_DATA;

typedef struct  { 
  TCPA_PAYLOAD_TYPE Payload;
  TCPA_SECRET       AuthData;
  TCPA_NONCE        TpmProof;
  TCPA_DIGEST       StoredDigest;
  UINT32            DataSize;
  UINT8             *Data;
} TCPA_SEALED_DATA;

typedef struct  {
  TCPA_ALGORITHM_ID AlgId;
  TCPA_ENC_SCHEME   EncScheme;
  UINT16            Size;
  UINT8             *Data;
} TCPA_SYMMETRIC_KEY;

typedef struct  {
  UINT32 KeyLength;
  UINT8  *Key;
} TCPA_STORE_PRIVKEY;

typedef struct  {
  TCPA_PAYLOAD_TYPE   Payload;
  TCPA_SECRET         UsageAuth;
  TCPA_SECRET         MigrationAuth;
  TCPA_DIGEST         PubDataDigest;
  TCPA_STORE_PRIVKEY  PrivKey;
} TCPA_STORE_ASYMKEY;

typedef struct {
  TCPA_PAYLOAD_TYPE   Payload;
  TCPA_SECRET         UsageAuth;
  TCPA_DIGEST         PubDataDigest;
  UINT32              PartPrivKeyLen;
  TCPA_STORE_PRIVKEY  PartPrivKey;
} TCPA_MIGRATE_ASYMKEY;

typedef struct {
  TCPA_PAYLOAD_TYPE   Payload;
  TCPA_NONCE          TpmProof;
  TCPA_STORE_PRIVKEY  PrivKey;
} TCPA_MAINTENANCE_ASYMKEY;

typedef struct {
  TCPA_VERSION          Version;
  TCPA_KEY_USAGE        KeyUsage;
  TCPA_KEY_FLAGS        KeyFlags;
  TCPA_AUTH_DATA_USAGE  AuthDataUsage;
  TCPA_KEY_PARMS        AlgorithmParms;
  TCPA_DIGEST           PubkeyDigest;
  TCPA_NONCE            Data;
  BOOLEAN               ParentPCRStatus;
  UINT32                PCRInfoSize;
  UINT8                 *PCRInfo;
} TCPA_CERTIFY_INFO;

typedef struct {
  TCPA_VERSION        Version;
  UINT8               Fixed[4];
  TCPA_COMPOSITE_HASH DigestValue;
  TCPA_NONCE          ExternalData;
} TCPA_QUOTE_INFO;

typedef struct  {
  TCPA_VERSION  Ver;
  UINT32        Ordinal;
  UINT32        LabelSize;
  TCPA_PUBKEY   CaPubKey;
  TCPA_PUBKEY   IdentityPubKey;
  UINT8         IdentityLabel[1];
} TCPA_IDENTITY_CONTENTS;

typedef struct  {
  UINT32          AsymSize;
  UINT32          SymSize;
  TCPA_KEY_PARMS  AsymAlgorithm;
  TCPA_KEY_PARMS  SymAlgorithm;
  UINT8           *AsymBlob;
  UINT8           *SymBlob;
} TCPA_IDENTITY_REQ;

typedef struct  {
  TCPA_VERSION  Ver;
  UINT32        LabelSize;
  UINT32        IdentityBindingSize;
  UINT32        EndorsementSize;
  UINT32        PlatformSize;
  UINT32        ConformanceSize;
  TCPA_PUBKEY   IdentityKey;
  UINT8         *LabelArea;
  UINT8         *IdentityBinding;
  UINT8         *EndorsementCredential;
  UINT8         *PlatformCredential;
  UINT8         *ConformanceCredential;
} TCPA_IDENTITY_PROOF;

typedef struct {
  TCPA_SYMMETRIC_KEY  SessionKey;
  TCPA_DIGEST         IdDigest;
} TCPA_ASYM_CA_CONTENTS;

typedef struct  {
  UINT32          CredSize;
  TCPA_KEY_PARMS  Algorithm;
  UINT8           *Credential;
} TCPA_SYM_CA_ATTESTATION;


//
// TCPA Command Ordinals
//
// Ordinals are 32 bit values. The first byte contains reserved values that 
// serve as flag indicators.
//
//                          1                   2                   3
//       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//      |T|V| Reserved  |   Command ordinal                             |
//      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//Where:
// T is TPM/TSS command. When 0 the command is a TPM command, when 1 the 
//    command is a TSS command
// V is TCPA/Vendor command. When 0 the command is TCPA defined, when 1 
//    the command is vendor defined.
// All reserved area bits are set to 0.
//
// The following masks are created to allow for the quick definition of the 
// commands
//
#define TCPA_ORDINAL        0x00000000
#define TCPA_TSS_COMMAND    0x80000000
#define TCPA_VENDOR_COMMAND 0x40000000

//
// Ordinals that are also found in TCPA Spec.
// 
#define TPM_ORD_OIAP                      10
#define TPM_ORD_OSAP                      11
#define TPM_ORD_ChangeAuth                12
#define TPM_ORD_TakeOwnership             13
#define TPM_ORD_ChangeAuthAsymStart       14
#define TPM_ORD_ChangeAuthAsymFinish      15
  
#define TPM_ORD_Extend                    20
#define TPM_ORD_PcrRead                   21
#define TPM_ORD_Quote                     22
#define TPM_ORD_Seal                      23
#define TPM_ORD_Unseal                    24
#define TPM_ORD_DirWriteAuth              25
#define TPM_ORD_DirRead                   26
  
#define TPM_ORD_UnBind                    30
#define TPM_ORD_CreateWrapKey             31
#define TPM_ORD_LoadKey                   32
#define TPM_ORD_GetPubKey                 33
#define TPM_ORD_EvictKey                  34
  
#define TPM_ORD_CreateMigrationBlob       40
#define TPM_ORD_ReWrapKey                 41
#define TPM_ORD_ConvertMigrationBlob      42
#define TPM_ORD_AuthorizeMigrationKey     43
#define TPM_ORD_CreateMaintenanceArchive  44
#define TPM_ORD_LoadMaintenanceArchive    45
#define TPM_ORD_KillMaintenanceFeature    46
#define TPM_ORD_LoadManuMaintPub          47
#define TPM_ORD_ReadManuMaintPub          48
  
#define TPM_ORD_CertifyKey                50
  
#define TPM_ORD_Sign                      60

#define TPM_ORD_GetRandom                 70
#define TPM_ORD_StirRandom                71
  
#define TPM_ORD_SelfTestFull              80
#define TPM_ORD_SelfTestStartup           81
#define TPM_ORD_CertifySelfTest           82
#define TPM_ORD_ContinueSelfTest          83
#define TPM_ORD_GetTestResult             84
  
#define TPM_ORD_Reset                     90
#define TPM_ORD_OwnerClear                91
#define TPM_ORD_DisableOwnerClear         92
#define TPM_ORD_ForceClear                93
#define TPM_ORD_DisableForceClear         94
  
#define TPM_ORD_GetCapabilitySigned       100
#define TPM_ORD_GetCapability             101
#define TPM_ORD_GetCapabilityOwner        102
  
#define TPM_ORD_OwnerSetDisable           110
#define TPM_ORD_PhysicalEnable            111
#define TPM_ORD_PhysicalDisable           112
#define TPM_ORD_SetOwnerInstall           113
#define TPM_ORD_PhysicalSetDeactivated    114
#define TPM_ORD_SetTempDeactivated        115
  
#define TPM_ORD_CreateEndorsementKeyPair  120
#define TPM_ORD_MakeTPMIdentity           121
#define TPM_ORD_ActivateTPMIdentity       122
#define TPM_ORD_ReadPubek                 124
#define TPM_ORD_OwnerReadPubek            125
#define TPM_ORD_DisablePubekRead          126
  
#define TPM_ORD_GetAuditEvent             130
#define TPM_ORD_GetAuditEventSigned       131
  
#define TPM_ORD_GetOrdinalAuditStatus     140
#define TPM_ORD_SetOrdinalAuditStatus     141
#define TPM_ORD_GetOrdinalStatusSigned    142
  
#define TPM_ORD_Terminate_Handle          150
#define TPM_ORD_Init                      151
#define TPM_ORD_SaveState                 152
#define TPM_ORD_Startup                   153
#define TPM_ORD_SetRedirection            154
  
#define TPM_ORD_SHA1Start                 160
#define TPM_ORD_SHA1Update                161
#define TPM_ORD_SHA1Complete              162
#define TPM_ORD_SHA1CompleteExtend        163
  
#define TPM_ORD_FieldUpgrade              170

#endif  // _EFI_TCPA_H_
