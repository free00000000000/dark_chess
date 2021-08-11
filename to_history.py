
with open('test_file/test.txt', 'r') as f:
  lines = f.read().split('\n')

result = []
gen = ''
start = False
for i in lines:
  if i[:2] == '7 ': 
    start = True
    result.append(i)

  elif start and i[:2] in ['10', '11']:
    result.append(i)

  elif i[:2] == '12':
    gen = i
result.append(gen)

with open('test_file/test1.txt', 'w') as f:
  f.write('\n'.join(result))