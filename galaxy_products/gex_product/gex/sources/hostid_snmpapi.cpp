// Only for PC platform (Windows 98 and higher.
// Used to read Ethernet board MAC address to create a host ID

#ifdef _WIN32

#include <windows.h>
#include <snmp.h>
#ifndef _INC_SNMP
SNMPAPI
WINSNMPAPI
SnmpUtilOidCpy(
    OUT AsnObjectIdentifier *DstObjId,
    IN  AsnObjectIdentifier *SrcObjId
    )
{
  DstObjId->ids = (UINT *)GlobalAlloc(GMEM_ZEROINIT,SrcObjId->idLength * 
          sizeof(UINT));
  if(!DstObjId->ids){
    SetLastError(1);
    return 0;
  }

  memcpy(DstObjId->ids,SrcObjId->ids,SrcObjId->idLength*sizeof(UINT));
  DstObjId->idLength = SrcObjId->idLength;

  return 1;
}


VOID
WINSNMPAPI
SnmpUtilOidFree(
    IN OUT AsnObjectIdentifier *ObjId
    )
{
  GlobalFree(ObjId->ids);
  ObjId->ids = 0;
  ObjId->idLength = 0;
}

SNMPAPI
WINSNMPAPI
SnmpUtilOidNCmp(
    IN AsnObjectIdentifier *ObjIdA,
    IN AsnObjectIdentifier *ObjIdB,
    IN UINT                 Len
    )
{
  UINT CmpLen;
  UINT i;
  int  res;

  CmpLen = Len;
  if(ObjIdA->idLength < CmpLen)
    CmpLen = ObjIdA->idLength;
  if(ObjIdB->idLength < CmpLen)
    CmpLen = ObjIdB->idLength;

  for(i=0;i<CmpLen;i++){
    res = ObjIdA->ids[i] - ObjIdB->ids[i];
    if(res!=0)
      return res;
  }
  return 0;
}

VOID
WINSNMPAPI
SnmpUtilVarBindFree(
    IN OUT RFC1157VarBind *VarBind
    )
{
  BYTE asnType;
  // free object name
  SnmpUtilOidFree(&VarBind->name);

  asnType = VarBind->value.asnType;

  if(asnType==ASN_OBJECTIDENTIFIER){
    SnmpUtilOidFree(&VarBind->value.asnValue.object);
  }
  else if(
        (asnType==ASN_OCTETSTRING) ||
        (asnType==ASN_RFC1155_IPADDRESS) ||
        (asnType==ASN_RFC1155_OPAQUE) ||
        (asnType==ASN_SEQUENCE)){
    if(VarBind->value.asnValue.string.dynamic){
      GlobalFree(VarBind->value.asnValue.string.stream);
    }
  }

  VarBind->value.asnType = ASN_NULL;

}

#endif
#endif
