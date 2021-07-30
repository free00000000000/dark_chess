
with open('test_file/test.txt', 'r') as f:
  lines = f.read().split('\n')

result = []
for i in lines:
  if i[:2] in ['7 ', '10', '11']:
    result.append(i)

with open('test_file/test1.txt', 'w') as f:
  f.write('\n'.join(result))