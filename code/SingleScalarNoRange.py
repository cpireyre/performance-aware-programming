# ========================================================================
# LISTING 16
# ========================================================================

def SingleScalarNoRange(Input):
    Sum = 0
    for Value in Input:
        Sum += Value
    return Sum

xs = list(range(4096))
print(SingleScalarNoRange(xs)) # 8386560

# ________________________________________________________
# Executed in   45.40 millis    fish           external
#    usr time   32.88 millis    0.22 millis   32.66 millis
#    sys time   10.92 millis    1.67 millis    9.25 millis

