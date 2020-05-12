# https://laszukdawid.com/2017/02/04/halton-sequence-in-python/

def next_prime():
    def is_prime(num):
        "Checks if num is a prime value"
        for i in range(2,int(num**0.5)+1):
            if(num % i)==0: return False
        return True
 
    prime = 3
    while(1):
        if is_prime(prime):
            yield prime
        prime += 2

def vdc(n, base=2):
    vdc, denom = 0, 1
    while n:
        denom *= base
        n, remainder = divmod(n, base)
        vdc += remainder/float(denom)
    return vdc

def halton_sequence(size, dim):
    seq = []
    primeGen = next_prime()
    next(primeGen)
    for d in range(dim):
        base = next(primeGen)
        seq.append([vdc(i, base) for i in range(size)])
    return seq

N = 32
for p in zip(*halton_sequence(N, 3)):
    print("\tsum += dist < texture(shadowDepth, pos + radius*vec3({:f},{:f},{:f})).x ? 1.0 : 0.0;"
          .format(p[0]-0.5, p[1]-0.5, p[2]-0.5))
print("\tsum *= 1.0 / {}.0;".format(N))
