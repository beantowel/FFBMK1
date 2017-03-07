import re
filename='origin.txt'
foutname=filename[:-4]+"_count.txt"
file=open(filename)
fileout=open(foutname,'w')
try:
    all_the_text=file.readlines()
finally:
    file.close()

strs=[
    'Packet Type:.*',
    'Effect Type:.*',
    'Effect Block Index:.*',
    'Cmd:\w*',
    'Loop.*'
]
sets=[]
for i in range(0,len(strs)):
    sets.append([])
for i in all_the_text: #range(0,len(all_the_text)):
    for j in range(0,len(strs)):
        key=i
        regex=strs[j]
        matcher=re.findall(regex,key)
        for k in matcher:
            sets[j].append(k)

    exMatcher=re.findall("ID:\w+",key)
    exMatcher2=re.findall("Cmd:\w+",key)
    if len(sets[0])>0 and len(exMatcher) > 0:
        sets[0][-1]=sets[0][-1]+' --> '+exMatcher[0]+' --> '+exMatcher2[0]
for i in sets:
    i=sorted(list(set(i)))
    for j in i:
        fileout.write(str(j)+'\n')
    fileout.write('\n')
