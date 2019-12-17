import random
import matplotlib.pyplot as plt

a = []
ans = []
count = 0
with open('addresses-locality.txt', 'r', encoding='utf-8') as f:
    # for i in range(10000):
    #     a[i] = random.randint(-5000, 5000)
    #     f.write(str(id(a[i])) + '\n')

    lines = f.readlines()
    for i, line in enumerate(lines):
        tline = line.split('\n')[0]
        if tline in ans:
            count += 1
        ans.append(tline)
        # if i > 10:
        #     break
# for i in range(10):
# 	print(ans[i])

# ## 绘图时下面注释掉
# random.shuffle(ans)
# with open("addresses-localityShuffle.txt", "w", encoding='utf-8') as f:
#     for i in range(len(ans)):
#         f.write(str(ans[i]) + '\n')


for i in range(len(ans)):
    a.append((int(ans[i]) & 0xFFFF) >> 8)
plt.scatter(range(len(ans)), a)
plt.show()   
