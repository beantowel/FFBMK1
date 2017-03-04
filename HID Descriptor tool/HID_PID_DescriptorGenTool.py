from HID_PID_Descriptor_Definitions import *
def Byte_Size(x):
	'''ByteSize for Short Items
	return 0,1,2,or4'''
	x=int(x)
	if x>=-0x7F-1 and x<=0x7F:
		return 1
	if x>=-0x7FFF-1 and x<=0x7FFF:
		return 2
	if x>=-0x7FFFFFFF-1 and x<=0x7FFFFFFF:
		return 4
def u_Byte_Size(x):
	'''ByteSize for Short Items
	return 0,1,2,or4'''
	x=int(x)
	if x<=0xFF:
		return 1
	if x<=0xFFFF:
		return 2
	if x<=0xFFFFFFFF:
		return 4
def toComplementBytes(x,size):
	x=int(x)
	if x<0:
		x=x+2**size
	return hex(x)
def SplitToLittleEndien(data,size):
	datalist=[]
	lsize=len(data)-2
	while(lsize!=size*2): #0xN-->0x0N
		data="0x0"+data[2:]
		lsize=len(data)-2
	for i in range(0,lsize,2):
		datalist.insert(0,"0x"+data[2+i:4+i])
	return datalist
def ShortItem(prefix,x,isSign):
	'''construct short item by prefix'''
	global bytecount
	if isSign:
		size=Byte_Size(x)
	else:
		size=u_Byte_Size(x)
	pre=hex(prefix+size)
	if len(pre)==3: #0xN-->0x0N
		pre="0x0"+pre[2:]
	compleSize=size*8
	if prefix==0x54: #Unit_Exponent exception
		compleSize=4 #one nibble
	splitData=toComplementBytes(x,compleSize)
	data=SplitToLittleEndien(splitData,size)
	if prefix==0xc0: #End_Collection exception
		bytecount+=1
		return "0xc0",[]
	bytecount=bytecount+1+len(data)
	return pre,data
# print(toComplementBytes(-65535))
# print(Local_Items)
fileIn="HID_Descriptor_Input.rptDsc"
fileOut=open("HID_DescriptorGen.out",'w')
lines=open(fileIn).readlines()
bytecount=0
#descriptor parser
for line in lines:
	line=line.expandtabs(tabsize=4)
	copyline=line
	line=line.lstrip()
	match=0
	for set1 in HID_Items:
		for item in set1:
			if line.find(item)==0:
				match=1
				break
		if match>0:
			break
	if match==0:
		continue
	prefix=set1[item]
	pos=line.find(item) #got item,to match data
	try:
		data=line[pos+len(item)+1:]
		try:
			posBracket=data.find(")") #int data
			data=int(data[:posBracket])
			printBytes=ShortItem(prefix,data,True)
		except:
			match2=0
			for set2 in HID_Constants: #defined constant data
				for cons in set2:
					if data.find(cons)==0:
						match2=1
						break
				if match2>0:
					break
			if match2==0:
				raise 'error match cons'
			data=set2[cons]
			printBytes=ShortItem(prefix,data,False)
	except:
		data=0 #defalut data
		printBytes=ShortItem(prefix,data,True)
	fileOut.write(printBytes[0]+",")
	for temp in printBytes[1]:
		fileOut.write(temp+",")
	for nspace in range(len(printBytes[1]),4):
		fileOut.write("     ")
	fileOut.write(" //"+copyline)
fileOut.write("//Total:"+str(bytecount)+" Bytes")
fileOut.close()
