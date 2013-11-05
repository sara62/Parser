fp = open("simpleAdd.txt","rb")

lines = [0] # Index with line number into list (starts at 1)

cl = 0
mark = True
while True:
    t = fp.read(1)
    if t == '': break
    if mark and ord(t) not in [13,10]:
        lines.append(cl); mark = False
    if t == '\n':
        mark = True
    cl += 1

for line in lines:
    fp.seek(line)
    print "{0: ^07} {1: ^07} {2}".format(line,lines.index(line),fp.readline()[:-1])