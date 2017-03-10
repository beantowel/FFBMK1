import re
import copy
from HID_PID_Descriptor_Definitions import *

def StatMachine(mainItem,value):
    global Outputs,structName,enumName

    if mainItem=='Collection':
        try:
            sN=lcalStat['Usage'].pop()
        except:
            print('error',lcalStat)
        structName.append(sN)
        Outputs.append('struct _'+sN+'{') #struct begins
    if mainItem=='End_Collection':
        sN=structName.pop()
        Outputs.append('} '+sN+';') #struct ends

    if mainItem in ['Input','Output','Feature']:
        if value=='IOF_Array':
            enumV=[] #enum values
            mini=glStat['Logical_Minimum']
            maxi=glStat['Logical_Maximum']
            for i in range(int(mini),int(maxi)+1):
                usage=lcalStat['Usage'].pop()
                usage=usage+'='+str(i)+','
                enumV.append(usage)

            enumT=structName[-1]+'_Enum'
            enumtype=glStat['Report_Size']
            enumtype='uint'+str(enumtype)+'_t' #ARM style typedef            
            enumName.append(['enum '+enumT+' : '+enumtype,copy.copy(enumV)])
            for i in range(0,int(glStat['Report_Count'])):
                Outputs.append(enumT+' '+enumT.lower()+'_'+str(i)+';')
        if value=='IOF_Variable':
            cnt=glStat['Report_Count']
            usage=[]
            for i in range(0,int(cnt)):
                usage.append(lcalStat['Usage'].pop())
            usage.reverse() #reverse to get appropriate order
            uType=glStat['Report_Size']
            if int(glStat['Logical_Minimum'])>=0:
                uType='uint'+str(uType)+'_t' #ARM style typedef
            else:
                uType='int'+str(uType)+'_t'
            for u in usage:
                Outputs.append(uType+' '+u.lower()+';')
        # if value=='IOF_Constant':
        #     gg
        # if value=='IOF_IOF_Defalut_Items':
        #     gg

fileIn     = "HID_Descriptor_Input.rptDsc"
fileOut    = open("HID_DataStructure.out",'w')
lines      = open(fileIn).readlines()
glStat     = {} #global Status
lcalStat   = {} #local Status
mainStat   = {} #main Status
status     = [glStat,lcalStat,mainStat]
Outputs    = []
structName = []
enumName   = []

strStart=False
strEnd=False
for line in lines:
    if line.find('Start Copy Data Structure Input')>=0:
        strStart=True
    if line.find('End Copy Data Structure Input')>=0:
        strEnd=True
    if not(strStart) or strEnd:
        continue
    #escape inputs

    line=line.expandtabs(tabsize=4)
    key=line
    pos=line.find('//') #remove comments
    if pos>=0:
        key=line[:pos]
    print('------',key) #echo

    for regex in Global_Items:
        item=re.findall('\\b'+regex+'\\b',key)
        value=re.findall('\(.*\)',key)
        for m in item:
            glStat[m]=value[0][1:-1]
            if m in ['Report_ID','Unit','Unit_Exponent']:
                Outputs.append('//'+m+':'+value[0][1:-1])
            break

    for regex in Local_Items:
        item=re.findall('\\b'+regex+'\\b',key)
        value=re.findall('\(.*\)',key)
        for m in item:
            try:
                lcalStat[m].append(value[0][1:-1])
            except:
                lcalStat[m]=[value[0][1:-1]]
            break

    for regex in Main_Items:        
        item=re.findall('\\b'+regex+'\\b',key)
        value=re.findall('\(.*\)',key)
        for m in item:
            StatMachine(m,value[0][1:-1])
            break

enumVis=[] #delete repeat definition
for enum in enumName:
    if enum[0] in enumVis:
        continue
    enumVis.append(enum[0])
    fileOut.write(enum[0]+'{'+'\n')
    for usage in enum[1]:
        fileOut.write(usage+'\n')
    fileOut.write('};\n\n')

cnt=0 #seperate each struct
for line in Outputs:
    if line.find('struct')>=0:
        if cnt==0:
            fileOut.write('\n')
        cnt+=1
    if line.find('}')>=0:
        cnt-=1
    fileOut.write(line+'\n')

fileOut.close()
