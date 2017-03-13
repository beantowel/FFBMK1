import re
def StatMachine(mainItem):
    global DataStruct,clcCnt
    print(DataStruct,clcCnt)
DataStruct=[]
clcCnt=0
DataStruct={}
#DataStruct['a']=['b']
try:
    DataStruct['a'].append('c')
except:
    DataStruct['a']=['c']
print(DataStruct)
StatMachine('s')
A=[1,2,3,4]
A.append(5)
print(A.pop())
print(A.pop())
print(A.pop())
print(A.pop())
print('\b'+'abc')

s='a abc abcd'
m=re.findall("\\ba.*",s)
print(m)

list1=['1','2','3']
print('list1:')
for i in list1:
    print(i)
list1.reverse()
if '3' in list1:
    print(list1)
for i in list1:
    print(i)
