################################################
# Example polynomial function
################################################
import polystate as ps
import math
import cmath
import numpy as np
from scipy.special import sph_harm
import letters
import zfrm

pi=math.pi

def none(t1,t2):
    return np.array([t1,t2]).astype(complex)

def poly_giga_1(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = 30 * (t1**2 * t2)
        cf[1] = 30 * (t1 * t2**2)
        cf[2] = 40 * (t1**3)
        cf[3] = 40 * (t2**3)
        cf[4] = -25 * (t1**2)
        cf[5] = -25 * (t2**2)
        cf[6] = 10 * (t1 * t2)
        cf[9] = 100 * (t1**4 * t2**4)
        cf[11] = -5 * t1
        cf[13] = 5 * t2
        cf[24] = -10
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_10(t1, t2):
    try:
        n = 120
        cf = np.zeros(n, dtype=np.complex128)
        re1, im1 = t1.real, t1.imag
        re2, im2 = t2.real, t2.imag
        
        for k in range(n):
            cf[k] = (100 * (re1 + im2) * ((k+1)/10)**2) * np.exp(1j * (re2 * (k+1) / 20)) + \
                    (50 * (im1 - re2) * np.sin((k+1) * 0.1 * im2)) * np.exp(-1j * (k+1) * 0.05 * re1)
        
        cf[29] = cf[29] + 1000j
        cf[59] = cf[59] - 500
        cf[89] = cf[89] + 250 * np.exp(1j * (t1 * t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_100(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(71):
            z = t1 * np.cos((i+1)*t2/15) + t2 * np.sin((i+1)*t1/15)
            phi = cmath.phase(z)
            r = np.abs(z)
            cf[i] = r * np.exp(1j * phi) ** (i+1) + (-1)**(i+2) * (i+1)**2
        
        cf[:30] = cf[:30] * (np.abs(t1) * np.abs(t2)) ** np.arange(1, 31)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_101(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(71):
            cf[i] = (t1/(i+2))**(i+1) + (t2/(i+2))*(2j*(i+1))
        
        even_indices = np.array([1,3,5,7,9,11,13,15,17,19])
        cf[even_indices] = cf[even_indices]*(t1+2*t2)
        
        third_indices = np.array([2,5,8,11,14,17,20,23,26,29])
        cf[third_indices] = cf[third_indices]*(t1-2*t2)
        
        cf[4:35] = cf[4:35] + 2*t1
        cf[35:66] = cf[35:66] - 2*t2
        cf[66:] = np.log(np.abs(cf[66:])).real + np.sum(cf[4])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_102(t1, t2):
    try:
        n = int(ps.poly.get("n") or 71)
        m = int(np.floor(n/2)+1)
        cf = np.zeros(n, dtype=np.complex128)
        if np.abs(t2)>0:
            cf[:(m-1)] = np.arange(1, m) * np.sin(t1) / np.abs(t2)
        else:
             cf[:(m-1)] = np.arange(1, m) * np.sin(t1)
        cf[m-1] = np.log(np.abs(t1 + t2) + 1)
        if np.abs(t1)>0:
            cf[m:] = np.arange(m-1, 0, -1) * np.cos(t2) / np.abs(t1)
        else:
            cf[m:] = np.arange(m-1, 0, -1) * np.cos(t2)
        nested_pattern = (-1)**np.arange(1, n+1)
        cf = cf * nested_pattern
        return cf.astype(np.complex128)
    except Exception as e:
        print(f"poly_giga_102 error: {e}")
        return np.zeros(0, dtype=np.complex128)

def poly_giga_103(t1, t2):
    try:
        n = int(ps.poly.get("n") or 71)
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(n):
            cf[k] = ((-1)**(k+1)) * ((k+1) / (t1 * np.abs(t1))) * np.abs(np.sin((k+1) * t2))
            if (k+1) % 2 == 0:
                cf[k] = cf[k] + np.abs(t1)**(k+1) * np.cos(t2)**(k+1)
            else:
                cf[k] = cf[k] + np.abs(t2)**(k+1) * np.sin(t1)**(k+1)
            if (k+1) % 3 == 0:
                cf[k] = cf[k] * cmath.phase(t1 + k+1)
            elif (k+1) % 5 == 0:
                cf[k] = cf[k] * cmath.phase(t2 - (k+1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_104(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        powers = ((np.arange(1, 71)**2 + t1 * 1j * t2)).real
        cf[1:] = 1 / (1 + powers)
        cf[0] = cf[1] + 100 * np.abs(t1 + t2)
        cf[2::2] = cf[2::2] * np.conj(t1 + t2)
        cf[3::2] = cf[3::2] * np.abs(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_105(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        powerValues = np.abs(t1 * t2) * np.arange(1, 71)
        cf[0] = (t1.real*t1.imag)/(t2.real*t2.imag) - (cmath.phase(t1)/cmath.phase(t2)) + np.log(np.abs(t1*t2+1))
        cf[1] = (t1.real*t2.imag)/(t2.real*t1.imag) + (cmath.phase(t1)/cmath.phase(t2)) - np.log(np.abs(t1*t2+1))
        
        for n in range(2, 71):
            cf[n] = np.sin(powerValues[n-2])*np.cos(powerValues[n-2]) + cf[n-1] + cf[n-2]
        
        cf[24:50] = 1 * np.abs(t1) * np.arange(51, 25, -1)
        cf[50:] = -1 * np.abs(t2) * np.arange(1, 22)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_106(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.abs(t1 + t2) ** 2
        cf[1] = t1.real * t2.imag - t2.real * t1.imag
        cf[2] = np.abs(t1) * np.abs(t2) ** 2 - np.abs(t1 - t2) ** 2
        
        for k in range(3, 36):
            cf[k] = (np.sin(k * t1 * t2) + np.cos(k * (t1 + t2))) / np.abs(np.sin(((k + 1) / 2) * (t1 - t2)) + np.cos(((k + 1) / 2) * (t1 + t2))) ** 2
        
        cf[36] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        
        for k in range(37, 71):
            cf[k] = (t1 * t2).real + (t1 * t2).imag / np.abs(t1 - t2) + np.log(np.abs(t1 * t2) + 1) / np.abs(t1 + t2)
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_107(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 * t2 - 1
        cf[4] = (t1**5 - t2**5) / 5
        cf[12] = (t1**7 + t2**7) / np.abs(t1*t2)
        cf[16] = np.log(np.abs(t1 + t2) + 1) * np.sin(t1*t2)
        
        for i in [22,30,42,52]:
            cf[i] = ((t1**i - t2**(70-1j))/i).real
            
        cf[66] = ((np.cos(t1)* np.sin(t2)**2)*((np.abs(t1)*abs(t2))**0.5)).imag - t1
        cf[70] = ((np.cos(t2)**2 * np.sin(t1)) / (np.log(np.abs(t1*t2)+1))).real
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_108(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = 100 * (t1 + t2**2)
        cf[1] = 90 * (t1**2 + t2)
        cf[2] = 80 * (t1**3 - t2**2)
        cf[3] = 70 * (t1**4 + t2**3)
        cf[4] = 60 * (t1**5 - t2**4)
        cf[5] = 50 * (t1**6 + t2**5)
        cf[6] = 40 * (t1**7 - t2**6)
        cf[7] = 30 * (t1**8 + t2**7)
        cf[8] = 20 * (t1**9 - t2**8)
        cf[9] = 10 * (t1**10 + t2**9)
        
        for k in range(10, 71):
            v = np.sin(k * np.log(np.abs(cf[k-1]+1))) + np.cos(k * np.log(np.abs(t1*t2+1)))
            cf[k] = np.conj(v) / np.abs(v)
            
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_109(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = 100 * (t1 + t2)
        cf[1] = t1.real ** 2 + t2.imag ** 2
        cf[2] = cmath.phase(t1) + cmath.phase(t2)
        cf[3] = np.abs(t1) / np.abs(t2)
        cf[4] = np.abs(t1 - t2)
        cf[5] = np.sin(t1) * np.cos(t2)
        
        for i in range(6, 70):
            cf[i] = np.abs(cf[i-1] / (i+1)) + cf[i-6] + np.log(np.abs(t1 + t2) + 1)
            
        cf[70] = np.prod(cf[np.array([0,10,20,30,40,50,60])]) / (np.abs(t1 + t2) + 1)
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_11(t1, t2):
    try:
        n = 40
        cf = np.zeros(n, dtype=np.complex128)
        m = int(5*abs(t1 + t2) % 17) + 1
        modular_values = np.arange(n) % m
        
        for k in range(n):
            scale_factor = modular_values[k]
            cf[k] = scale_factor * np.exp(1j * np.pi * (k+1) / (m + t1 + t2))
            
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def one_ball(t1,t2):
    try:
        v=np.array([
            1/(t1+t2+4),
            2/(t1+t2+4)
        ]).astype(complex)
        cf = np.exp(1j * np.pi * v)
        return cf.astype(np.complex128)
    except:
        return np.zeros(2,dtype=np.complex128)
    
def two_balls(t1,t2):
    try:
        v=np.array([
            1/(t1+t2+4),
            2/(t1+t2+4),
            4/(t1+t2+4)
        ]).astype(complex)
        cf = np.exp(1j * np.pi * v)
        return cf.astype(np.complex128)
    except:
        return np.zeros(3,dtype=np.complex128)

def three_balls(t1,t2):
    try:
        v=np.array([
            1/(t1+t2+4),
            2/(t1+t2+4),
            4/(t1+t2+4),
            8/(t1+t2+4)
        ]).astype(complex)
        cf = np.exp(1j * np.pi * v)
        return cf.astype(np.complex128)
    except:
        return np.zeros(3,dtype=np.complex128)

def four_balls(t1,t2):
    try:
        v=np.array([
            1/(t1+t2+4),
            2/(t1+t2+4),
            4/(t1+t2+4),
            8/(t1+t2+4),
            16/(t1+t2+4)
        ]).astype(complex)
        cf = np.exp(1j * np.pi * v)
        return cf.astype(np.complex128)
    except:
        return np.zeros(3,dtype=np.complex128)        


def p11a(t1, t2):
    try:
        n  = 40
        a  = np.abs( t1 + t2 ) / 2
        m  = int( ( 10 * a) % 13 ) + 3
        v  = np.arange(n) / ( max(m,3) + t1 + t2 )
        uc = np.exp(1j * np.pi * v)
        sf = np.arange(n) % (m * 2)
        cf = sf * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def p11a_v1(t1, t2):
    try:
        n  = 40
        a  = np.abs( t1 + t2 ) / 2
        m  = int( ( 5 * a) % 21 ) + 3
        v  = np.pow(np.linspace(0, 1, n),0.75) / ( max(m,3) + t1 + t2 )
        uc = np.exp(1j * 50 * np.pi * v)
        sf = np.arange(n) % (m+10)
        cf = sf * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def p11a1(t1, t2):
    try:
        n  = 40
        a  = np.abs( t1 + t2 ) / 2
        m  = int( ( 10 * a ) % 13 )
        v0 = np.linspace(0, 1, n)
        v  = n * v0 / ( m + 3 + t1 + t2 )
        uc = np.exp(1j * np.pi * v)
        sf = np.arange(n) % (m * 2)
        cf = sf * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def p11a2(t1, t2):
    try:
        n  = 40
        a  = np.abs( t1 + t2 ) / 2
        m  = int( ( 101 * a ) % 17 )
        v0 = np.linspace(0, 1, n)
        v  = n * v0 / ( m + 3 + t1 + t2 )
        uc = np.exp(1j * np.pi * v)
        sf = np.arange(n) % (m * 2)
        cf = sf * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def p11b(t1, t2):
    try:
        n  = 40
        a  = np.abs( t1 + t2 ) / 2
        m  =  int(10 * a) % 17 + 1
        v  = np.linspace(0, 1, n)
        cc = 1 # circle count
        sf = ( (n-1) * v + 1.1 ).astype(int) % int( cc * m  ) 
        oc = 1.0 # outside cicle relative size, smaller value = larger circles
        denom = m + t1 + t2 
        if np.abs(denom) <1 :
            denom=1
        u  = n * np.power(v,oc) / denom # min value = 3 -1 -1 = 1
        d  = 1 # circle diameter
        uc = np.exp(1j * d * np.pi * u)
        cf = sf * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def p11b1(t1, t2):
    try:
        n  = 3
        v = np.linspace(0, 1, n)
        denom =  t1 + t2 + 3 
        # denom =  t1**2 - t2**2 + t1 * t2 + t1 + t2 + 4
        if np.abs(denom)<1 :
            denom = denom / np.abs(denom)
        u  = v / denom 
        cf = np.exp(1j * np.pi * u)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def p11b2(t1, t2):
    try:
        n  = 71
        v = np.linspace(0, 1, n)
        denom =  t1 + t2 + 3 
        if np.abs(denom)<1 :
            denom = denom / np.abs(denom)
        u  = 7 * n * np.power(v,3) / denom 
        uc = np.exp(1j * np.pi * u)
        sf = (np.arange(n) * 1 ) % (int(101 * np.abs( t1 + t2 )) % 51 + 1)
        cf = sf * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def p11b2_v1(t1, t2):
    try:
        n  = 71
        v = np.linspace(0, 1, n)
        denom =  t1 + t2 + 3 
        if np.abs(denom)<1 :
            denom = denom / np.abs(denom)
        u  = 7 * n * np.power(v,15) / denom 
        uc = np.exp(1j * np.pi * u)
        sf = (np.arange(n) + 1 ) % (int(4583 * np.abs( t1 + t2 )) % 71 + 1)
        cf = (sf+1) * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def p11b2_v2(t1, t2):
    try:
        n  = 11
        v = np.linspace(0, 1, n)
        denom =  t1 + t2 + 3 
        if np.abs(denom)<1 :
            denom = denom / np.abs(denom)
        u  = 7 * n * np.power(v,15) / denom 
        uc = np.exp(1j * np.pi * u)
        sf = (np.arange(n) + 1 ) % (int(4583 * np.abs( t1 + t2 )) % 11 + 1)
        cf = (sf+1) * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def p11b2_v3(t1, t2):
    try:
        n  = 11
        v = np.linspace(0, 1, n)
        denom =  t1 + t2 + 3 
        if np.abs(denom)<1 :
            denom = denom / np.abs(denom)
        u  = 7 * n * np.power(v,15) / denom 
        uc = np.exp(1j * np.pi * u)
        sf = np.power(np.cos(2 * np.pi * (v+0.5j)),3) * (n-1)
        ## % (int(4583 * np.abs( t1 + t2 )) % 11 + 11)
        cf = (sf+1) * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def p11b2_v4(t1, t2):
    try:
        n  = 11
        v = np.linspace(0, 1, n)
        denom =  t1 + t2 + 3 
        if np.abs(denom)<1 :
            denom = denom / np.abs(denom)
        u  = 7 * n * np.power(v,15) / denom 
        uc = np.exp(1j * np.pi * u)
        sf = np.power(np.cos(2 * np.pi * (v-0.5j)),3) * (n-1)
        cf = (sf+1) * uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def p11b2_v5(t1, t2):
    try:
        n  = 11
        v1 = np.exp(1j*2*np.pi*np.linspace(t1, t2, n))
        v2 = np.exp(1j*2*np.pi*np.linspace(t1+t2, t1*t2, n))
        v = v1 + 1j * v2
        denom =  t1 + t2 + 3 
        if np.abs(denom)<1 :
            denom = denom / np.abs(denom)
        u  = n * np.power(v,1) / denom 
        uc = np.exp(1j * np.pi * u)
        cf = uc
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
            
def p11b3(t1, t2):
    try:
        t = t1 + t2 # balls with holes
        a = np.abs(t1+t2) / 2 # number between 0 and 1 
        m = int( a * 251 ) % 37 # integer 0 - 10
        v = (np.arange(11)+1) / (t+4)
        p1 = int( 7 * a * len(v) ) % len(v) # pick a random spot
        v[p1] = (p1+1)/(t + np.abs(t)*2 + 1 + m) # stick m in it
        p2 = int( 619 * a * len(v) ) % len(v) # pick a random spot
        v[p2] = (p2+1)/(t + np.abs(t)*2 + 1 + m//2) # stick m in it
        cf = np.exp(1j * np.pi * v)
        return cf.astype(np.complex128)
    except:
        return np.zeros(4, dtype=np.complex128)

def poly_giga_110(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(71):
            cf[k] = np.sin((k+1)*t1)*np.cos((k+1)*t2) / (np.abs((k+1)*t1)+1)**(1/(k+1))
        
        cf[:10] = cf[:10]**3
        cf[60:] = cf[60:] / cf[10:21]
        cf[30:40] = cf[30:40] * np.conj(t2)
        cf[40:50] = cf[40:50] * np.conj(t1)
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_111(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**2 + 2*t2**2 + 1
        cf[1] = t1*t2/2 + 2*t1**3 - 3*t2 + 1j
        
        for k in range(2, 71):
            cf[k] = (7*t1 + t2)/((k+1)*t1 + (k+1)*t2) + np.sin((k+1)*t1) - np.cos((k+1)*t2) + np.abs(t1) * (k+1)**2 - np.abs(t2) * (k+1)**3 + 1
            
        cf[39:50] = cf[39:50]*np.cos(t1+t2) + 1j*np.sin(t1-t2)
        cf[59:] = cf[59:]*np.log(np.abs(t1+t2)+1) - 1j*np.log(np.abs(t1-t2)+1)
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def poly_giga_112(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(71):
            j = 70 - i
            cf[i] = ((np.real(t1) + np.imag(t1) * j) / np.abs(t2 + (i+1))) * np.sin(np.angle(t1 + t2 * (i+1))) + np.log(np.abs(t1 * t2) + 1) * np.cos(2 * np.pi * (i+1) / 71)
        
        cf[cf == 0] = np.real(t1) ** 2 - np.imag(t1) * np.imag(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_113(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = (t1**2 + t2**2) / ((t1 + t2)**2)
        cf[1] = 1j * (np.real(t1) + np.real(t2)) / ((np.imag(t1) + np.imag(t2))**2)
        cf[2:10] = [(t1*t2)**(i+1) / (np.abs(t1 - t2) * (i+1)) for i in range(8)]
        cf[10:21] = np.sin((t1 + t2)**2) * (np.real(t1)**3 + np.imag(t2)**3)
        cf[21:31] = np.cos((t1 - t2)**2) * (np.imag(t1)**3 + np.real(t2)**3)
        
        for j in range(31, 43):
            cf[j] = cf[j-1] / np.abs(cf[j-1] + cf[j-2])
            
        cf[43:50] = [np.log(np.abs(t1 + 1j * t2)) * (i+1)**2 for i in range(7)]
        cf[50:55] = [(t1 + t2)**5 / (np.abs(t1 - t2) * (5-i))**2 for i in range(5)]
        cf[55:60] = [(t1 - t2)**4 / (np.abs(t1 * 1j * t2) * (5-i))**2 for i in range(5)]
        cf[60:65] = [(t1 * 1j * t2)**3 / (np.abs(t1 - t2) * (5-i)) for i in range(5)]
        cf[65:70] = [(t1**2 - t2**2) * (i+1)**2 / (np.abs(t1)**4 + np.abs(t2)**4) for i in range(5)]
        cf[70] = (t1 * t2 * (t1 - t2)) / (np.abs(t1 + t2) * np.abs(t1 - t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_114(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(71):
            cf[i] = np.real(t1) * np.real(t2) * ((i+1)**2)/(np.exp(np.abs(t1)*1j)) + np.imag(t1) * np.imag(t2) * ((i+1)**3)/(np.exp(np.abs(t2)*1j))
        
        cf[1::2] = cf[1::2] * (-1)
        p = np.arange(1, 72)
        cf[p**2 <= 71] = cf[p**2 <= 71] + 1j*abs(t1)*abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_115(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:35] = np.sin(t1+1j) * np.arange(1, 36) * np.abs(t2)**2
        cf[35:71] = np.log(np.abs(t1)) * np.arange(36, 72) * np.abs(t2)
        
        cf[0:10] = cf[0:10] * t1 + t2
        cf[60:71] = cf[60:71] * (t1 + 1j*t2)
        cf[10:20] = cf[10:20]/t1**2
        cf[20:30] = cf[20:30] * np.conj(t2)**3
        cf[30:40] = cf[30:40] * t1 + 2*np.real(t2)
        cf[40:50] = cf[40:50]/(t1 + 1j*t2 - 1)
        cf[50:60] = cf[50:60] * (3*t1 - 1j*t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_116(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(71):
            cf[i] = (np.abs(t1) + np.abs(t2)) * np.sin((i+1) / (t1 * t2)) * (i+1)**(1/3)
        
        cf[0:23] = cf[0:23] * ((t1 + t2) / np.abs(t1 - t2))**2
        cf[23:47] = cf[23:47] * np.log(np.abs((t1 + t2)**3) + 1)
        cf[47:71] = cf[47:71] * np.log(np.abs((t1 - t2)**3) + 1)
        
        cf[0] *= 2
        cf[22] *= 2
        cf[46] *= 2
        cf[70] *= 2
        
        cf[1:22] = cf[1:22] / np.real(t1)
        cf[23:46] = cf[23:46] / np.imag(t2)
        cf[2:22] = cf[2:22] / np.abs(cf[2:22])
        cf[24:46] = cf[24:46] / np.abs(cf[24:46])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_117(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = 1000 * t1**2 * t2**2 + 500 * t1 + 100
        cf[1] = 500 * t1**2 - 1000 * t2 + 300
        cf[2] = 300 * t1**3 - 200 * t2**2 + 100 * t1 - 200
        cf[3] = 200 * t1**4 + 100 * t2**3 - 10 * t1**2 + 20 * t2
        
        for i in range(4, 71):
            cf[i] = ((-1)**(i+1)) * (t1**(i+3) + 2 * t2**(i+2)) / np.log(np.abs(t1)+1) / (i+1)
        
        for j in range(5, 71, 5):
            cf[j] = cf[j] + np.sin(t1 * t2) * 200
            
        for k in [6, 20, 34, 48, 62]:
            cf[k] = cf[k] + np.cos(t1 * t2)**2 * 500
            
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_118(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(35):
            cf[i] = (i+1)*(t1 + (i+1)*1j*t2)**(1/(i+1))
            cf[70] = np.conj(cf[i])
        
        cf[35] = 2*t1 + 3*abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_119(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        
        for n in range(1, 71):
            cf[n] = ((n+1)**4 * (np.abs(t1)*abs(t2))**((n+1)/70) * np.sin(np.angle(t1)*np.angle(t2))*(n+1)) / (np.abs(t1+t2)**2 * (np.log(np.abs(t1+t2)+1))**(1/(n+1)))
        
        cf[0] = (np.abs(t1*t2)**3 - np.abs(t1+t2)**3 + np.abs(t1-t2)**3) / ((np.log(np.abs(t1+t2)+1))**3 * np.angle(t1+t2))
        
        cf[2:5] = cf[2:5] * np.conj(t1)
        cf[5:10] = cf[5:10] * np.conj(t2)
        cf[12:19] = cf[12:19] * np.abs(t1) * np.abs(t2)
        cf[19:30] = cf[19:30] * (np.imag(np.sin(t1*t2)) + np.real(np.cos(t1*t2))) / (np.abs(t1+t2))
        cf[30:50] = cf[30:50] * (np.imag(t1)*np.log(np.abs(t2)+1) - np.real(t2)*np.log(np.abs(t1)+1)) * np.abs(t1+t2)
        cf[50:71] = cf[50:71] * (np.real(t1)*np.log(np.abs(t2)+1) - np.imag(t2)*np.log(np.abs(t1)+1)) / (np.abs(t1+t2)**2)
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def poly_giga_12(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = 2
        cf[2] = 50 * (t1**3)
        cf[4] = 50 * (t2**3)
        cf[6] = -30 * (t1**2)
        cf[8] = -30 * (t2**2)
        cf[10] = 100 * (t1 * t2)
        cf[12] = 50 * (t1**2 * t2)
        cf[14] = 50 * (t1 * t2**2)
        cf[19] = -75 * (t1**3 * t2**3)
        cf[20] = 3.5 * t2
        cf[24] = -2 * t1
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_120(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(71):
            if (k + 1) % 2 == 0:
                cf[k] = (np.log(np.abs(t1)+1)**(k+1) + np.log(np.abs(t2)+1)**(71-(k+1))) * np.sin((k+1)*t1 + (71-(k+1))*t2)
            else:
                cf[k] = (np.log(np.abs(t1)+1)**(k+1) - np.log(np.abs(t2)+1)**(71-(k+1))) * np.cos((k+1)*t1 - (71-(k+1))*t2)
        
        r = np.abs(t1) * np.abs(t2)
        for k in range(49, 71):
            cf[k] = cf[k] * (r ** (k+1-50))
        
        for k in range(14, 35):
            cf[k] = cf[k] * 2 * (r ** (71 - (k+1)))
        
        cf = cf.real + 1j * np.conj(cf).imag
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_121(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1.real + 1000j
        cf[1] = np.log(1+abs(t1+t2)) * 1000
        
        for k in range(2, 35):
            cf[k] = (-1)**k * (np.real(t1**k) + np.imag(t2**k)) * 1000 / (k ** 2)
        
        for k in range(35, 70):
            cf[k] = (-1)**(k+1) * (np.abs(t1)**(70-k) + np.abs(np.sin(t1+t2))) / (k ** 2)
        
        cf[70] = np.abs(t1) + np.cos(np.angle(t2)) * 1000
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_122(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(35):
            cf[k] = (t1 + 1j * t2) ** (k+1) + np.log(np.abs(t1+(k+1)*t2)+1) * np.real(t1 * t2)
            cf[70 - k] = (k+1) * (t1 - 1j * t2) ** (k+1) - np.log(np.abs(t2-(k+1)*t1)+1) * np.imag(t1 * t2)
        
        cf[35] = 100 * np.abs(t1) * np.abs(t2)
        cf[36] = 200 * np.angle(t1) * np.angle(t2)
        cf[37:71] = cf[0:34] - cf[37:71]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_123(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1*t2 - 1j * np.abs(t2 - t1)
        
        for ranges in [(1,6), (6,11), (11,16), (16,21), (21,26), (26,31), 
                      (31,36), (36,41), (41,46), (46,51), (51,56), (56,61),
                      (61,66), (66,71)]:
            start, end = ranges
            for k in range(start, end):
                if k % 20 < 5:
                    cf[k] = cf[k-1] + np.sin((k+1) * t1) + np.cos((k+1) * t2)
                elif k % 20 < 10:
                    cf[k] = cf[k-1] + np.log(np.abs(t1 - (k+1))) - np.log(np.abs(t2 - (k+1)))
                elif k % 20 < 15:
                    cf[k] = cf[k-1] - np.sin((k+1) * t1) - np.cos((k+1) * t2)
                else:
                    cf[k] = cf[k-1] - np.log(np.abs(t1 - (k+1))) + np.log(np.abs(t2 - (k+1)))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_124(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(70):
            cf[k] = t1**(k+1) * t2**(k+1)
        
        cf[0] = np.log(np.abs(t1 + t2)) + 1
        cf[1] = np.log(np.abs(t1 * t2)) + 1
        cf[3] = np.abs(t1)**2 + np.abs(t2)**2
        cf[5] = np.abs(t1)**3 - np.abs(t2)**3
        cf[7] = np.abs(t1)**4 + np.abs(t2)**4
        cf[9] = t1**5 - t2**5
        cf[19] = np.abs(t1) * np.sin(np.angle(t2))
        cf[29] = np.abs(t2) * np.real(t1)
        cf[39] = np.abs(t1) * np.imag(t2)
        cf[49] = np.abs(t2) * np.angle(t1)
        cf[59] = np.abs(t1 * t2) * np.cos(np.angle(t2))
        cf[69] = np.abs(t2 * t1) * np.real(t1)
        cf[70] = np.sum(cf[:70])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_125(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2 + 1
        cf[1:10] = np.real(t1)**2 + np.imag(t2)**2
        cf[10:20] = np.real(t2)**2 + np.imag(t1)**2
        cf[20:30] = np.abs(t1 * t2)**2
        cf[30:40] = np.abs(t1 + t2)**2
        cf[40:50] = np.abs(t1) * np.abs(t2)
        cf[50:60] = np.angle(t1) + np.angle(t2)
        cf[60:70] = np.sin(t1 + t2)
        cf[70] = np.cos(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_126(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = np.real(t1) + np.imag(t2)
        cf[1] = np.angle(t1)
        cf[2] = np.abs(t2)
        cf[3] = np.sin(t1) + np.cos(t2)
        cf[4:10] = np.linspace(1, 2, 6)
        cf[10] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        
        for i in range(11, 51):
            cf[i] = cf[i-1] * np.sin(i * cf[i-2] + np.abs(cf[i-3])) + cf[i-4]
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_127(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79])
        
        for i in range(len(primes)):
            cf[i] = t1**primes[i] / (t2 + np.abs(t1) * np.abs(t2))
            cf[i+len(primes)] = t2**primes[i] / (t1 + np.abs(t1) * np.abs(t2))
        
        cf[39] = np.sin(t1)/t2 + np.sin(t2)/t1
        cf[40:43] = np.log(np.abs(t1+t2))+1
        cf[43] = t1*t2 + np.imag(t1)*np.imag(t2)
        cf[44] = t1*t2 - np.real(t1)*np.real(t2)
        cf[45:50] = np.real(t1)*np.imag(t2) - np.imag(t1)*np.real(t2)
        cf[50] = np.abs(t1)*abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_128(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        triangleNums = np.cumsum(np.arange(1, 51))
        cf[0] = t1 + 3 * t2
        
        for k in range(1, 51):
            cf[k] = triangleNums[k-1] * (t1 + t2 * np.log(np.abs(t1) + 1))**(k) + \
                    triangleNums[k-1] * (t2 + t1 * np.log(np.abs(t2) + 1))**(k)
        
        cf[42] = np.real(np.abs(t1)) + np.imag(np.abs(t2))
        cf[20] = np.real(np.abs(t2)) + np.imag(np.abs(t1))
        cf[31] = np.real(np.abs(t1*t2)) + np.imag(np.conj(t1*t2))
        cf[27] = 2 * np.real(t1 - t2) + 2 * np.imag(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_129(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        n = np.arange(1, 51)
        cf[0] = t1 + t2
        cf[1] = np.sin(t1) + np.cos(t2)
        cf[2:51] = n * np.log(np.abs(t1 + t2 * np.real(np.conj(t1 * t2)))) * \
                   (np.abs(t1 * t2)**n) * (t1 * np.real(np.conj(t2)))**n
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_13(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[1] = 100 * (t1**4)
        cf[3] = 100 * (t2**4)
        cf[5] = 80 * (t1**3 * t2)
        cf[7] = 80 * (t1 * t2**3)
        cf[9] = 1 * t1
        cf[11] = -1 * t2
        cf[13] = 5 * (t1**2 * t2**2)
        cf[17] = -0.5 * (t1**5)
        cf[18] = -0.5 * (t2**5)
        cf[22] = 2.3 * (t1**2 - t2**2)
        cf[24] = 10 * (t1**3 - t2**3)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_130(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = t1 * t2
        for i in range(1, 51):
            cf[i] = (cf[i-1] + (i+1) * t1) / (1 + (i+1) * t2)
        for j in range(25):
            cf[2*j+1] = np.abs(cf[2*j+1] * t1**(j+1) / t2**(j+1))
        cf[29] = np.real(t1 + t2) + np.imag(t1 - t2)
        cf[39] = np.angle(t1) * np.angle(t2)
        cf[49] = np.log(np.abs(t1*t2) + 1)
        cf[50] = cf[0] + np.real(t1**2 + t2**2) - np.imag(t1**2 - t2**2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_131(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = 1
        for i in range(1, 51):
            cf[i] = np.sin(i * t1) * np.cos(i**2 * t2) + np.log(np.abs(cf[i-1])**(i+1) * (i+1))
        cf[24] = t1 / t2 + np.abs(t1) * np.abs(t2)
        for i in range(29, 50):
            cf[i] = cf[i] + t1 + (t2 / (i+1)) - t1 * (i+1) - t2 * np.abs(cf[24]) / (t1 + 0.5 * t2)
        cf[49] = np.sum(cf[24:49]) - t2
        cf[50] = np.sum(cf[48:50]) + t1
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_132(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241])
        for i in range(50):
            cf[i] = (primes[i]*t1 + 1j*t2**(i+1))/(1 + np.abs(t1))**(i+1)
        cf[50] = np.sum(cf[:50])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_133(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = np.real(t1**2 - t2**2)
        primes = np.array([2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97])
        for k in range(2, 25):
            cf[k] = np.imag(cf[k-1] * primes[k-2]) * np.angle(t1) * np.abs(t2)
        for k in range(25, 50):
            cf[k] = np.abs(cf[k-1] * primes[k-25]**2) * np.angle(t2) * np.real(t1)
        cf[50] = np.sum(cf) + np.sin(np.real(t2))*np.log(np.abs(t1)+1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_134(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = np.real(t1 * t2) + np.imag(t2) * np.real(t1)
        cf[1] = np.abs(t1 * t2) * np.cos(np.angle(t1 + t2))
        for i in range(2, 51):
            cf[i] = cf[i-2] * np.abs(cf[i-1]) * np.sin(np.angle(t1 + t2))
        cf[50] = np.log(np.abs(t1 * t2)) + cf[0] + cf[1]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_135(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0:5] = [1, t1, t1**2, t1**3, t1**4]
        cf[5:10] = [1, t2, t2**2, t2**3, t2**4]
        
        cf[10:15] = [1, np.exp(1j * t1), np.exp(2j * t1), np.exp(3j * t1), np.exp(4j * t1)]
        cf[15:20] = [1, np.exp(1j * t2), np.exp(2j * t2), np.exp(3j * t2), np.exp(4j * t2)]
        
        cf[20:30] = [1, np.real(t1+t2), np.imag(t1+t2), np.real(t1*t2), np.imag(t1*t2), 
                     np.real(t1+t2)**2, np.imag(t1+t2)**2, np.real(t1*t2)**2, np.imag(t1*t2)**2, np.abs(t1+t2)]
        cf[30:40] = np.arange(1, 11) * np.abs(t1) * np.abs(t2)
        
        cf[40:50] = [1, np.log(np.abs(t1) + 1), np.log(np.abs(t2) + 1), np.log(np.abs(t1 + t2) + 1),
                     np.log(np.abs(t1 * t2) + 1), np.angle(t1), np.angle(t2), np.abs(t1), np.abs(t2), np.angle(t1 + t2)]
        cf[50] = np.abs(t1+t2) * np.angle(t1*t2)
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_136(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1**2 - 2*t2 + 5
        cf[2] = np.conj(t1) * t2 + 7
        cf[3] = t2**2 - t1 + 11
        cf[4] = np.abs(t1 + t2) + 13
        cf[5] = np.angle(t1) * np.angle(t2) + 17
        cf[6] = t1 * t2 - 19
        cf[7] = t1**3 + t2**3 + 23
        cf[8] = np.sin(t1) + np.cos(t2) + 29
        cf[9] = np.log(np.abs(t1 + t2) + 1) + 31
        cf[10] = t1**2 - t2**2 + 37
        cf[11] = np.conj(t2) * t1 + 41
        cf[12] = np.imag(t1) * np.real(t2) - 43
        cf[13] = t1 * np.conj(t2) + 47
        cf[14] = np.abs(t1 - t2) + 53
        cf[15] = t1**4 - t2**4 + 59
        cf[16] = 61 - 5*t1*t2
        cf[17] = 67 + np.abs(t1**2 + t2**2)
        cf[18] = 71 + t1**5 + t2**5
        cf[19] = 73 - np.angle(t1) * np.angle(t2)
        cf[20] = 79 + np.abs(t1**3 + t2**3)
        cf[21] = 83 - t1**6 + t2**6
        cf[22] = 89 + np.sin(t1 + t2)
        cf[23] = np.abs(np.real(t1) * np.imag(t2)) + 97
        cf[24] = 101 + t1*t2**2
        cf[25] = 103 - np.conj(t1) * np.real(t2)
        cf[26] = 107 + t1**7 - t2**7
        cf[27] = 109 + np.abs(np.conj(t1-t2))
        cf[28] = 113 - np.abs(t1**2 - t2**2)
        cf[29] = 127 + (t1**8 * t2**8)
        cf[30] = t1 - t2 + np.abs(t1*t2) + 131
        cf[31] = 137 + np.angle(t1**2) - np.angle(t2**2)
        cf[32] = 139 - t1**9 + t2**9
        cf[33] = np.log(np.abs(t1*t2) + 1) + 149
        cf[34] = 151 + (np.abs(t1) + np.abs(t2))**2
        cf[35] = np.sin(2*t1) - np.cos(2*t2) + 157
        cf[36] = np.log(np.abs(t1-t2) + 1) + 163
        cf[37] = 167 + np.real(t1**3) - np.imag(t2**3)
        cf[38] = 173 - (t1**2 * t2**2)**1.5
        cf[39] = 179 + np.angle(t1*t2) + 1j
        cf[40] = 181 - np.conj(t1**3 - t2**3)
        cf[41] = 191 + np.abs(t1) * np.abs(t2)
        cf[42] = 193 - np.abs(np.real(t1) + np.imag(t2))
        cf[43] = 197 + np.sin(t1**2 + t2**2)
        cf[44] = 199 - t1*t2**3
        cf[45] = t1*np.imag(t2) + 211
        cf[46] = np.abs(t1**4 + t2**4) + 223
        cf[47] = 227 - np.conj(t1**2) * np.conj(t2**2)
        cf[48] = 229 + np.sin(t1*t2) - np.cos(t1-t2)
        cf[49] = 233 + t1**9 - t2**9
        cf[50] = 239 - np.abs(np.conj(t1**2+t2**2))
        cf[51] = 241 + t1**3 + t2**3
        cf[52] = t1**10 + t2**10 + 251
        cf[53] = t1*t2*np.real(t1+t2) - 257
        cf[54] = np.abs(t1-t2) - 263
        cf[55] = t1**11 - t2**11 + 269
        cf[56] = 271 + np.abs(t1*t2**2 - t2**3)
        cf[57] = 277 + np.sin(t1**3 - t2**3)
        cf[58] = 281 - np.conj(t1**2*t2)
        cf[59] = np.conj(t1**5 + t2**5) + 283
        cf[60] = np.angle(t1**3 * t2**3) + 293
        cf[61] = 307 - np.sin(t1*t2 + 1j)
        cf[62] = np.abs(t1**6 + t2**6) + 311
        cf[63] = 313 - np.cos(t1**3-t2**3)
        cf[64] = np.angle(t1*t2) + 317
        cf[65] = np.real(t1**2-t2**2) - 331
        cf[66] = 337 + np.abs(t1**6 * t2**6)
        cf[67] = 347 - np.abs(t1**4 - t2**4)
        cf[68] = 349 + np.sin(np.conj(t1-t2))
        cf[69] = 353 - np.cos(t1+t2**2)
        cf[70] = np.abs((t1+t2)**3 - 359)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_14(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = 200 * (t1**3 * t2**2)
        cf[4] = 200 * (t1**2 * t2**3)
        cf[6] = 50 * (t1**4)
        cf[8] = 50 * (t2**4)
        cf[10] = -100 * (t1**3)
        cf[12] = -100 * (t2**3)
        cf[14] = 10 * (t1**2 - t2**2)
        cf[16] = 20 * (t1 - t2)
        cf[18] = 0.1 * (t1**5)
        cf[20] = 0.1 * (t2**5)
        cf[22] = 0.05 * (t1 * t2)
        cf[24] = -10
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_15(t1, t2):
    try:
        cf = np.zeros(18, dtype=np.complex128)
        cf[1] = 200 * (t1**5 + t2**5)
        cf[3] = 100 * (t1**4 - t2**4)
        cf[5] = 80 * (t1**6)
        cf[7] = 80 * (t2**6)
        cf[9] = 2 * t1
        cf[11] = -2 * t2
        cf[13] = 5 * (t1**3 * t2**3)
        cf[17] = 5
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_16(t1, t2):
    try:
        cf = np.zeros(22, dtype=np.complex128)
        cf[12] = 250 * (t1**5 - t2**3)
        cf[17] = 200 * (t1**4 * t2**4)
        cf[8] = 80 * (t1**2 * t2 - t2**2)
        cf[6] = -60 * (t1**3)
        cf[4] = 40 * (t2**3)
        cf[2] = 15 * (t1 - 0.5*t2)
        cf[3] = -20 * (t1*t2)
        cf[1] = 5 * t2
        cf[0] = -10
        cf[21] = -30 * (t1**6 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_17(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        theta1 = np.angle(t1)
        theta2 = np.angle(t2)
        scale1 = 1 + 0.5 * np.sin(5*theta1 - 3*theta2)
        scale2 = 1 + 0.3 * np.cos(7*theta1 + 2*theta2)
        
        high_deg_index = 20 if np.sin(theta1 - theta2) > 0 else 15
        cf[high_deg_index] = 300 * (t1**7 * t2**9) * scale1
        
        if np.cos(theta1 + theta2) < 0:
            cf[18] = 250 * (t1**4 * t2**7) * scale2
        else:
            cf[12] = 250 * (t1**4 * t2**7) * scale2
            
        scale3 = 1 + 0.4 * np.sin(theta1 + 2*theta2)
        cf[8] = 80 * (t1**5 - t2) * scale3
        cf[6] = -100 * (t1**2 * t2**2) * scale3
        cf[4] = -20 * (t1**3 + t2) * scale3
        
        scale4 = 1 + 0.2 * np.cos(2*theta1 - theta2)
        cf[2] = -5 * (t1**2 - t2**2) * scale4
        cf[1] = (t2**3 - t1) * scale4
        cf[0] = -5
        
        perturb_scale = 0.5 + 0.5 * np.sin(3*theta1) * np.cos(4*theta2)
        cf[24] = (t1**4 - t2**4) * perturb_scale
        cf[22] = -0.5 * (t1**9 + t2**9) * perturb_scale
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_18(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        theta1 = np.angle(t1)
        theta2 = np.angle(t2)
        
        base_scale = 1000 * np.exp(0.5 * np.sin(10*theta1 - 7*theta2))
        secondary_scale = 500 * (np.cos(12*theta1 + 15*theta2))**3
        
        if np.sin(theta1 - theta2) > 0:
            toggle_scale = 2000 * np.sin(5*theta1)*np.cos(3*theta2)
        else:
            toggle_scale = -2000 * np.cos(4*theta1)*np.sin(2*theta2)
            
        if np.cos(theta1 + theta2) > 0.5:
            cf[20] = (t1**7 * t2**9) * base_scale * toggle_scale
        elif np.cos(theta1 + theta2) < -0.5:
            cf[18] = (t1**10 - t2**10) * secondary_scale * toggle_scale
        else:
            cf[12] = (t1**4 * t2**7 - t1**5) * base_scale * secondary_scale
            
        complex_scale = 300 * (np.sin(np.sin(3*theta1 + 4*theta2)))**2
        cf[8] = (t1**5 - t2)*complex_scale
        
        another_scale = 100 * np.exp(np.sin(theta1)*np.cos(theta2))
        cf[6] = -another_scale * (t1**2 * t2**2)
        
        sign_flip = 1 if (np.floor((theta1+theta2)*3) % 2) == 0 else -1
        cf[4] = sign_flip * 50 * (t1**3 + t2) * (np.sin(2*theta1 - theta2))
        cf[2] = -5 * (t1**2 - t2**2) * (10 * np.cos(5*theta2))
        cf[1] = (t2**3 - t1) * (200 * np.sin(3*theta1)*np.sin(theta2))
        cf[0] = -5
        
        cf[24] = (t1**4 - t2**4) * 100 * (np.cos(np.sin(theta1)*theta2)) * np.exp(np.cos(2*theta1 - 3*theta2))
        cf[22] = -10 * (t1**9 + t2**9) * (np.sin(7*theta1 - 8*theta2))**3
        cf[15] = 500 * (t1**6 - t2**3) * np.sin((theta1 + theta2)**2) * np.cos((theta1 - theta2)**2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_19(t1, t2):
    try:
        cf = np.zeros(90, dtype=np.complex128)
        cf[0] = t1 - t2
        for k in range(1, len(cf)):
            v = np.sin(k * cf[k-1]) + np.cos(k * t1)
            av = np.abs(v)
            cf[k] = 1j * v / av if np.isfinite(av) and av > 1e-10 else t1 + t2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_19_fixed(t1, t2):
    try:
        # t1=int(t1*40000.0)/40000.0
        # t2=int(t2*40000.0)/40000.0
        cf = np.zeros(90, dtype=np.complex128)
        cf[0] = t1 - t2
        for k in range(1, len(cf)):
            v = np.sin((k+1) * cf[k-1]) + np.cos((k+1) * t1)
            av = np.abs(v)
            if np.isfinite(av) and av > 1e-10:
                cf[k] = 1j * v / av
            else:
                cf[k]=t1+t2    
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_19_fixed1(t1, t2):
    try:
        res = 1000.0
        t1=int(t1*res)/res
        t2=int(t2*res)/res
        cf = np.zeros(90, dtype=np.complex128)
        cf[0] = t1 - t2
        for k in range(1, len(cf)):
            v = np.sin((k+1) * cf[k-1]) + np.cos((k+1) * t1)
            av = np.abs(v)
            if np.isfinite(av) and av > 1e-10:
                cf[k] = 1j * v / av
            else:
                cf[k]=t1+t2    
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def poly_giga_2(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[9] = 200 * (t1**2 + t2**2)
        cf[15] = 150 * (t1**3 * t2**5)
        cf[7] = -80 * (t1**4 - t2**2)     # z^7 term (index 8 means z^(8-1)=z^7)
        cf[5] = 50 * (t1**3 + t2)         # z^5 term (z^(6-1)=z^5)
        cf[2] = 20 * (t1 - t2)            # z^2
        cf[1] = -10 * (t1 * t2)           # z^1
        cf[0] = -5
        cf[19] = -30 * (t1**5 - t2**5)    # z^19
        cf[24] = 10 * (t1 * t2**3)        # z^24
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_20(t1, t2):
    try:
        cf = np.zeros(90, dtype=np.complex128)
        cf[0] = t1 + 1j * t2
        for k in range(1, len(cf)):
            v = np.sin(k * cf[k-1]) + np.cos(k * t1)
            av = np.abs(v)
            if np.isfinite(av) and av > 1e-10:
                cf[k] = 1j * v / av
            else:
                cf[k] = t1 + t2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_21(t1, t2):
    try:
        cf = np.zeros(50, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, len(cf)):
            v = np.sin(((k+3) % 10) * cf[k-1]) + np.cos(((k+1) % 10) * t1)
            av = np.abs(v)
            if np.isfinite(av) and av > 1e-10:
                cf[k] = v / av
            else:
                cf[k] = t1 + t2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_22(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 100
        cf[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
        cf[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
        cf[16] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
        cf[20] = -10
        cf[24] = 0.2j
        cf[25] = 0
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_23(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        indices = [0, 4, 12, 19, 20, 24]
        values = [1, 4, 4, -9, -1.9, 0.2]
        cf[indices] = values
        cf[6] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
        cf[8] = 100 * t1**3 + 100 * t1**2 + 100 * t2 - 100
        cf[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_24(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        indices = [0, 4, 12, 19, 20, 24]
        values = [1, 4, 4, -9, -1.9, 0.2]
        cf[indices] = values
        cf[6] = 100j * t2**3 + 100j * t2**2 - 100 * t2 - 100
        cf[8] = 100j * t1**3 + 100j * t1**2 + 100 * t2 - 100
        cf[14] = 100j * t2**3 - 100j * t2**2 + 100 * t2 - 100
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_25(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 100
        cf[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
        cf[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
        cf[16] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
        cf[20] = -10
        cf[24] = 0.2j
        cf[25] = 0
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_26(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 100
        cf[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
        cf[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
        cf[16] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
        cf[20] = -10
        cf[24] = 0.2
        cf[25] = 0
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_27(t1, t2):
    try:
        n = 12
        cf = np.zeros(n, dtype=np.complex128)
        cf[0:3] = [-100j, -100j, -100j]
        mid_indices = [n//2-2, n//2-1, n//2]
        cf[mid_indices] = 100 * np.roots([t1, t2, t1, 1])
        end_indices = [n-1, n-2, n-3]
        cf[end_indices] = 100 * np.roots([t2, t1, t2, 10j])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_28(t1, t2):
    try:
        n = 6
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 100 * t2**3 + 100j * t1**3
        cf[1] = 0
        # cf[n//2-1] = 150
        cf[int(n/2) - 1] = 150
        cf[n-2] = 0
        cf[n-1] = 40j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_29(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 150 * t2**3 - 150j * t1**2
        cf[1] = 0
        cf[n//2-1] = 100*(t1-t2)**1
        cf[n-2] = 0
        cf[n-1] = 10j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_3(t1, t2):
    try:
        n = 25
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 1
        cf[n-11] = np.exp(t1-t2)
        cf[n-10] = 0*np.exp(t1+t2)
        cf[n-9] = 1*np.exp(1j*t1)
        cf[n-8] = 1*np.exp(t1)
        cf[n-7] = 1*np.exp(-t1)
        cf[n-6] = 1*np.exp(-1j*t1)
        cf[n-2] = 1*np.exp(1j*t2)
        cf[n-1] = 1+1j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_30(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 150j * t2**2 + 100 * t1**3
        cf[n//2-1] = 150 * np.abs(t1 + t2 - 2.5 * (1j + 1))
        cf[n-1] = 100j * t1**3 + 150 * t2**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def safe_polyroot(coeffs):
    try:
        return np.roots(coeffs)
    except:
        return np.array([])

def poly_giga_31(t1, t2):
    try:
        n = 100
        cf = np.zeros(n, dtype=np.complex128)
        cf[0:3] = [-100j, 0, 0]
        pr1 = safe_polyroot([t1, t2, t1, 1])
        if len(pr1) == 3:
            cf[n//2-2:n//2+1] = 100 * pr1
        else:
            cf[n//2-2:n//2+1] = 100
        pr2 = safe_polyroot([t2, t1, t2, 10j])
        if len(pr2) == 3:
            cf[n-3:n] = 100 * pr2
        else:
            cf[n-3:n] = 100
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_32(t1, t2):
    try:
        n = 12
        cf = np.zeros(n, dtype=np.complex128)
        cf[0:3] = [-100j, -100j, -100j]
        mid_indices = [n//2-2, n//2-1, n//2]
        cf[mid_indices] = 100 * np.roots([t1, t2, t1, 1])
        end_indices = [n-1, n-2, n-3]
        cf[end_indices] = 100 * np.roots([t2, t1, t2, 10j])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_33(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        cf[0] = 11j * t1**4 + 13j * t1**3 + 17j * t1**2 + 19j * t1 + 23j
        cf[1] = 100j * t1**3 + 100j * t2**2 - 100 * t1 - 100
        cf[2] = 100 * t2**3 + 100j * t1**2 - 100j * t2 - 100j
        cf[3] = 100j * t1**3 + 100 * t2**2 - 100 * t1 - 100j
        cf[4] = -3
        cf[6] = 101 * t2**3 + 103 * t2**2 - 107 * t2 - 109
        cf[8] = 113 * t1**3 + 127 * t1**2 + 131 * t2 - 137
        cf[12] = 5
        cf[14] = 67 * t2**3 - 71 * t2**2 + 73 * t2 - 79
        cf[16] = -7
        cf[20] = 11
        cf[24] = -13
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_34(t1, t2):
    try:
        n = 120
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = -1
        cf[n//2-1] = 100 * t1 - 100j * t2
        cf[n-1] = 0.4
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_35(t1, t2):
    try:
        n = 120
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = -1
        cf[n//2-1] = 100 * t1 - 100j * t2
        cf[n-1] = 0.4
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_36(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        cf[0] = 1
        cf[1] = 100 * t1**3 + 100 * t2**2 - 100 * t1 - 100
        cf[2] = 100 * t2**3 + 100 * t1**2 - 100 * t2 - 100
        cf[4] = 4
        cf[6] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
        cf[8] = 100 * t1**3 + 100 * t1**2 + 100 * t2 - 100
        cf[12] = -8
        cf[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
        cf[19] = 16
        cf[20] = -32
        cf[24] = 64
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def poly_giga_37(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        cf[0] = 11j * t1**4 + 13j * t1**3 + 17j * t1**2 + 19j * t1 + 23j
        cf[1] = 100j * t1**3 + 100j * t2**2 - 100 * t1 - 100
        cf[2] = 100 * t2**3 + 100j * t1**2 - 100j * t2 - 100j
        cf[3] = 100j * t1**3 + 100 * t2**2 - 100 * t1 - 100j
        cf[4] = -3
        cf[6] = 101 * t2**3 + 103 * t2**2 - 107 * t2 - 109
        cf[8] = 113 * t1**3 + 127 * t1**2 + 131 * t2 - 137
        cf[12] = 5
        cf[14] = 67 * t2**3 - 71 * t2**2 + 73 * t2 - 79
        cf[16] = -7
        cf[20] = 11
        cf[24] = -13
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_38(t1, t2):
    try:
        n = 26
        cf1 = np.zeros(n, dtype=np.complex128)
        cf1[0] = 100
        cf1[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
        cf1[14] = 100 * t1**3 - 100 * t1**2 + 100 * t1 - 100
        cf1[16] = 100 * t1**3 + 100 * t1**2 - 100 * t1 - 100
        cf1[20] = -10
        cf1[24] = np.exp(0.2j * t2)
        cf1[25] = 0

        cf2 = np.zeros(n, dtype=np.complex128)
        cf2[0] = 100
        cf2[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
        cf2[14] = 100 * t2**3 - 100 * t2**2 + 100 * t2 - 100
        cf2[16] = 100 * t2**3 + 100 * t2**2 - 100 * t2 - 100
        cf2[20] = -10
        cf2[24] = 0.2j
        cf2[25] = 0

        result = (cf1 - 0.0001 * np.sum(np.abs(cf1))) * (cf2 + 1.5j * np.sum(np.abs(cf2)))
        return np.flip(result)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_39(t1, t2):
    try:
        cf = np.zeros(50, dtype=np.complex128)
        cf[np.array([0, 9, 19, 29, 39, 49])] = np.array([1, 2, -3, 4, -5, 6])
        cf[14] = 100 * (t1**2 + t2**2)
        cf[24] = 50 * (np.sin(t1) + 1j * np.cos(t2))
        cf[34] = 200 * (t1 * t2) + 1j * (t1**3 - t2**3)
        cf[44] = np.exp(1j * (t1 + t2)) + np.exp(-1j * (t1 - t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_4(t1, t2):
    try:
        n = 25
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 100
        cf[12] = 100 * t1**3 + 100 * t1**2 + 100 * t1 - 100
        cf[14] = 100 * t1**3 - 100 * t1**2 + 100 * t1 - 100
        cf[16] = 100 * t1**3 + 100 * t1**2 - 100 * t1 - 100
        cf[20] = -10
        cf[24] = np.exp(0.2j * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_40(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        cf[np.array([0, 6, 14, 19, 26, 34])] = [1, -2, 3, -4, 5, -6]
        cf[11] = 50j * np.sin(t1**2 - t2**2)
        cf[17] = 100 * (np.cos(t1) + 1j * np.sin(t2))
        cf[24] = 50 * (t1**3 - t2**3 + 1j * t1 * t2)
        cf[29] = 200 * np.exp(1j * t1) + 50 * np.exp(-1j * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_41(t1, t2):
    try:
        cf = np.zeros(60, dtype=np.complex128)
        cf[np.array([0, 9, 29, 49])] = [1, -5, 10, -20]
        cf[19] = 100 * np.exp(t1 + t2)
        cf[39] = 50 * (t1**2 * t2 + 1j * t2**2)
        cf[54] = np.exp(1j * t1) * np.exp(-1j * t2) + 50 * t1**3
        cf[59] = 300 * np.sin(t1 + t2) + 1j * np.cos(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_42(t1, t2):
    try:
        cf = np.zeros(50, dtype=np.complex128)
        cf[np.array([0, 7, 15, 31, 39])] = [1, -3, 3, -1, 2]
        cf[11] = 100j * np.exp(t1**2 + t2**2)
        cf[19] = 50 * (t1**3 + t2**3)
        cf[24] = np.exp(1j * (t1 - t2)) + 10 * t1**2
        cf[44] = 200 * np.sin(t1 + t2) + 1j * np.cos(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_43(t1, t2):
    try:
        cf = np.zeros(40, dtype=np.complex128)
        cf[np.array([0, 4, 14, 29])] = [1, -5, 10, -20]
        cf[19] = 100j * (t1**3 - t2**3)
        cf[9] = 50 * (t1**2 * t2 + 1j * t2**2)
        cf[24] = np.exp(1j * t1) + np.exp(-1j * t2)
        cf[34] = 200 * t1 * t2 * np.sin(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_44(t1, t2):
    try:
        cf = np.zeros(30, dtype=np.complex128)
        cf[np.array([0, 5, 11, 19])] = [1, 3, -2, 5]
        cf[9] = 100 * t1**3 + 50 * t2**2
        cf[14] = 50j * (t1.real - t2.imag)
        cf[24] = 200 * t1 * (t2 + 1) - 100j * t2
        cf[29] = np.exp(1j * t1) + t2**3
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_45(t1, t2):
    try:
        cf = np.zeros(50, dtype=np.complex128)
        cf[0] = 1
        cf[4] = 50 * np.exp(t1)
        cf[9] = 100 * (t2**2 - 1j * t1)
        cf[19] = 200 * np.exp(1j * t1**2) - 50 * np.exp(-1j * t2**3)
        cf[29] = 100 * t1 * t2**2 + 50j * t1**3
        cf[39] = np.exp(1j * (t1 + t2)) - 50 * np.sin((t1 - t2).imag)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_46(t1, t2):
    try:
        cf = np.zeros(40, dtype=np.complex128)
        cf[np.array([0, 7, 15, 23, 31])] = [1, -3, 5, -7, 2]
        cf[4] = 50 * (t1**2 - t2**3)
        cf[11] = 100j * (t1**3 + t2)
        cf[19] = np.exp(1j * t1) + np.exp(-1j * t2**2)
        cf[29] = 200 * np.sin(t1.real + t2.imag) - 50 * np.cos((t1 - t2).imag)
        cf[34] = np.exp(1j * t1**3) + t2**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def pr(x):
    return np.array(x, dtype=np.complex128)

def poly_giga_47(t1, t2):
    try:
        cf = np.zeros(30, dtype=np.complex128)
        cf[0:4] = pr([t1**3 - t2**2, 100 * t1, -50 * t2, 10j])
        cf[9:12] = pr([1, t1**2 - 1j * t2, -100])
        cf[14] = 50 * t1**3 - 20 * t2
        cf[24] = 200 * np.sin(t1.real + t2.imag) + 1j * np.cos(t1.imag - t2.real)
        cf[29] = np.exp(1j * t1) + t2**3
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def giga_47_old(t1, t2):
    cf = np.zeros(30, dtype=np.complex128)
    cf[0:4] = np.array([t1**3 - t2**2, 100 * t1, -50 * t2, 10j], dtype=np.complex128)
    cf[9:12] = np.array([1, t1**2 - 1j * t2, -100], dtype=np.complex128)
    cf[14] = 50 * t1**3 - 20 * t2
    cf[24] = 200 * np.sin(t1.real + t2.imag) + 1j * np.cos(t1.imag - t2.real)
    cf[29] = np.exp(1j * t1) + t2**3    
    return cf.astype(np.complex128)

def poly_giga_48(t1, t2):
    try:
        cf = np.zeros(40, dtype=np.complex128)
        cf[0:4] = pr([np.sin(t1) + np.cos(t2), 100 * t1**2, -50 * t2, 10j])
        cf[9:12] = pr([np.cos(t1.real) + np.sin(t2.imag), -1, t1**3 - t2**2])
        cf[19] = 50 * (t1**2 - t2**3)
        cf[29] = np.exp(1j * t1) + t2**2
        cf[34] = 200 * np.sin(t1.real + t2.imag) + 50 * np.cos((t1 - t2).imag)
        cf[39] = np.exp(1j * t1**3) + t2**3
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_49(t1, t2):
    try:
        cf = np.zeros(30, dtype=np.complex128)
        unstable_roots = pr([t1**4 - t2**3, 1j * t1 - t2, 50 * np.sin(t1 + t2), -100])
        cf[0:4] = unstable_roots
        cf[7:10] = pr([unstable_roots[0]**2, -unstable_roots[1], 1])
        cf[14] = 100j * (t1**2 + t2**3)
        cf[24] = 200 * np.sin(t1.real + t2.imag) + 50 * np.cos((t1 - t2).imag)
        cf[29] = np.exp(1j * t1**3) + t2**3
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_giga_5(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        cf[[0, 4, 12, 19, 20, 24]] = [1, 4, 4, -9, -1.9, 0.2]
        cf[6] = 100j * t2**3 + 100j * t2**2 - 100j * t2 - 100j
        cf[8] = 100j * t1**3 + 100j * t1**2 + 100j * t2 - 100j
        cf[14] = 100j * t2**3 - 100j * t2**2 + 100j * t2 - 100j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_50(t1, t2):
    try:
        n = 1000
        cf = np.zeros(n, dtype=np.complex128)
        w = 100
        p1 = 1 * t2**3 - 1 * t2**2 + 1 * t2 - 1
        p2 = 1 * t2**3 + 1 * t2**2 + 1 * t2 + 1
        p3 = 1 * t1**3 + 1 * t2**2 + 1 * t1 + 1
        p4 = 1 * t1**3 + 1 * t1**2 + 1 * t1 + 1
        p5 = 1 * t1**3 + 1 * t1**2 + 1 * t1 - 1
        
        cf[0] = 10
        idx_range = np.arange(int(n*0.25) - 5, int(n*0.25) + 1)
        cf[idx_range] = 1e8*w * np.array([1j*p1, p2, p1, p1*p3, p1-p3, p1+p3]) * np.abs(t1-t2)
        
        cf[n//2-2] = w * p1
        cf[n//2-1] = w * p2
        cf[n//2] = w * (p1+p4)
        cf[n//2+1] = w * p4
        cf[n//2+2] = w * 1j * p4
        
        idx_range = np.arange(int(n*0.75), int(n*0.75) + 6)
        cf[idx_range] = w * np.array([1j*p1, p2, p1, p1*p3, p1-p3, p1+p3]) * np.exp(1j*abs(t1)) * 100
        
        cf[n-5] = 4 * np.exp(0.3*t1) * np.abs(t2)
        cf[n-4] = 4 * np.exp(0.3*t2) * np.abs(t1-t2)
        cf[n-1] = 10j * np.exp(0.4j*t2) * np.abs(t1)
        cf[n-1] = 8j * np.abs(t2) * (1/(t1-2))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_51(t1, t2):
    try:
        n = 1000
        cf = np.zeros(n, dtype=np.complex128)
        p1 = 1 * t2**3 - 1 * t2**2 + 1 * t2 - 1
        p2 = 1 * t2**3 + 1 * t2**2 + 1 * t2 + 1
        p3 = 1 * t1**3 + 1 * t2**2 + 1 * t1 + 1
        p4 = 1 * t1**3 + 1 * t1**2 + 1 * t1 + 1
        p5 = 1 * t1**3 + 1 * t1**2 + 1 * t1 - 1
        
        pp = np.array([1j*p1, p2**2, p4, p1+p2+p3, 1j*p1, p1**9, p2**9, p1*p2*p3*p4, p1, p1*p3, p1-p3, p1+p3])
        
        for i in range(n):
            k = i % len(pp)
            cf[i] = pp[k]
            
        cf[0] = -0.1j
        if np.abs(cf[n-1]) < 1e-10:
            cf[n-1] = 1
            
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_52(t1, t2):
    try:
        n = 100
        cf = np.zeros(n, dtype=np.complex128)
        
        # Using numpy's roots function instead of R's polyroot
        cf[[0, 1, 2]] = [-100j, 0, 0]
        roots1 = np.roots([1, t1, t2, t1])
        roots2 = np.roots([10j, t2, t1, t2])
        
        cf[n//2-1:n//2+2] = 100 * np.resize(roots1, 3)
        cf[n-3:n] = 100 * np.resize(roots2, 3)
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_53(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 100 * np.sin(t1)**3 * np.cos(t2)**2
        cf[1] = 100 * np.exp(1j * (t1 + t2)) - 10 * (t1 - t2)**2
        cf[2] = t1*t2*(t1 - t2) / (np.abs(t1) + np.abs(t2) + 1)
        cf[4] = (t1*t2*np.exp(1j * (t1**2-t2**2)))**3
        cf[6] = np.sqrt(np.abs(t1)) - np.sqrt(np.abs(t2)) + 1j * np.sin(t1*t2)
        cf[7] = 50 * np.abs(t1 - t2) * np.exp(1j * np.abs(t1 + t2))
        cf[8] = t1-abs(t2) if t1.imag > 0 else t2-abs(t1)
        cf[9] = (1j*t1*t2)**(0.1*t1*t2)
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_54(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = t1.real * np.exp(1j * t2)
        cf[2] = (t1 * t2).imag * np.exp(-1j * t2.real)
        cf[4] = (t1.real + t2.imag)**2 + 10j
        cf[6] = (t2.imag**3) / t1.real - 1j
        cf[8] = (t1 * t2).real * np.exp(1j * ((t1 + t2).imag**2))
        cf[9] = np.sum(cf[0:9])
        cf[10] = np.prod(cf[0:10])
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(11, dtype=np.complex128)

def poly_giga_55(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        
        cf[0] = np.exp(1j * t1)
        cf[1] = (t1 + t2) * np.cos(t1) + 1j * np.sin(t2)
        cf[2] = t1**3 * t2**2 - 1j * t1**2 * t2**3
        cf[3] = np.log(t1 + 1j*t2)
        cf[4] = t1 * np.cos(t1) + t2 * np.sin(t2)
        cf[5] = t1**2 * t2 - t1 * t2**2
        cf[6] = 1j * t1**3 + t2**3
        cf[7] = (t1 + 1j * t2)**3 - t1 * t2
        cf[8] = t1 * t2 * (t1 - t2) * (t1 + t2)
        cf[9] = t1**3 * t2**2 * np.exp(1j * (t1 - t2))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(10, dtype=np.complex128)

def poly_giga_56(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = t1**2 - t2**2
        cf[2] = t1**3 + t2**3
        cf[3] = np.sin(t1) * np.cos(t2)
        cf[4] = np.exp(1j * (t1 - t2))
        cf[5] = np.log(np.abs(t1 + t2))
        cf[6] = t1**4 + 1j * t2**4
        cf[7] = (t1 * t2)**2
        cf[8] = (t1 + t2)/2
        cf[9] = t1**5 - t2**5
        cf[10] = np.exp(1j * (t1 * t2))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(11, dtype=np.complex128)

def poly_giga_57(t1, t2):
    try:
        cf = np.zeros(10, dtype=np.complex128)
        cf[0] = np.log(np.abs(t1 + t2)) * np.sin(t1 - t2)
        cf[1] = np.exp(t1.real) + np.exp(t2.imag)
        cf[2] = np.sqrt(np.abs(t1)) * np.cos(t2)
        cf[3] = np.sin((t1 + t2).imag**3) * np.exp((t1 - t2).real**2)
        cf[4] = np.tan((t1 * t2).imag) * np.cosh((t1 * t2).real)
        cf[5] = np.abs(t1 - t2) * np.sinh(np.angle(t1 + t2))
        cf[6] = (t1**3 - t2**2).imag * np.tan((t1 * t2).real)
        cf[7] = np.tanh(np.abs(t1) * np.abs(t2)) * np.sin(np.angle(t1 / t2))
        cf[8] = np.sign((t1 - t2).real) * np.cosh(np.angle(t1 * t2))
        cf[9] = np.arctan(1j * t1 / t2) + np.arcsinh(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(10, dtype=np.complex128)

def poly_giga_58(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = np.log(1j * t1**2 + 1)
        cf[1] = np.exp(1j * t1 * t2) + 1
        cf[2] = np.sin(t1 * t2) + t2
        cf[3] = np.cos(t1**3 + t2**2) * 1j
        cf[4] = t1 * t2 * (t1 - t2)
        cf[5] = np.sqrt(np.abs(t1 * t2)) * (t1 + t2)
        cf[6] = t1**3 * t2**3 * 1j
        cf[7] = (t2 - t1)/(t1 + t2) * 1j
        cf[8] = np.log(t1*t2) + np.sin(t1 + t2) * 1j
        cf[9] = 3**t1 * 2**t2
        return cf.astype(np.complex128)
    except:
        return np.zeros(10, dtype=np.complex128)

def poly_giga_59(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = 100 * (t1**4 - t2**4)
        cf[1] = -100j * (t1 * t2 * (t1**2 + t2**2))
        cf[2] = 100 * np.sqrt((t1**2 * t2**2).real)
        cf[3] = 100 * (t1 - t2).imag
        cf[4] = 100j * (t1**2 + t2**2)
        cf[5] = 100 * np.exp(1j * np.abs(t1 - t2))
        cf[6] = 100 * np.sin((t1 + t2).real)
        cf[7] = -100j * np.cos((t1 - t2).imag)
        cf[8] = 100 * (t1*t2 / np.abs(t1*t2))
        cf[9] = 100 * np.sqrt(t1.real * t2.real) - 100j * np.sqrt(t1.imag * t2.imag)
        cf[10] = 100 * np.exp(1j * (np.angle(t1) - np.angle(t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(10, dtype=np.complex128)

def poly_giga_6(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 150 * t2**3 - 150j * t1**2
        cf[1] = 0
        cf[n//2-1] = 100*(t1-t2)**1
        cf[n-2] = 0
        cf[n-1] = 10j
        return cf.astype(np.complex128)
    except:
        return np.zeros(10, dtype=np.complex128)

def poly_giga_60(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            cf[k] = np.sin(k+1)*t1/(1+abs(t2)) + np.cos(k+1)*t2/(1+abs(t1)) + np.sqrt(k+1)
        cf[0] = np.abs(t1)*abs(t2)
        cf[4] = np.angle(t1)*abs(t2)
        cf[9] = np.abs(t1)*np.angle(t2)
        cf[14] = np.abs(t1)*t2.real
        cf[19] = np.abs(t1)*t2.imag
        cf[24] = t1.real*abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_61(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.abs(t1 + t2)
        cf[1] = 2*t1.real*t2.imag
        cf[2] = np.angle(t1 + t2)
        cf[3] = np.conj(t1)*t2
        cf[4] = np.angle(t1)*np.angle(t2)
        
        for k in range(5, 21):
            cf[k] = np.abs(t1 + (-1)**(k+1)*t1**2/(k+1) + (-1)**(k+1)*t2**2/(k+1))
        
        cf[21] = cf[1] + cf[2] - cf[3] + cf[4]
        cf[22] = np.abs(cf[1]*cf[2]*cf[3]*cf[4])
        cf[23] = 1 + (np.conj(t1)*t2).real
        cf[24] = 1j + (np.conj(t1)*t2).imag
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_62(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0:5] = np.array([abs(t1 + t2)**(i+1) for i in range(5)])
        cf[5:10] = ((t1+2j*t2)**3).real * np.log(np.abs(np.conj(t1*t2)))
        cf[10:15] = ((t1-t2)**2).imag / np.angle(t1*t2)
        cf[15:20] = np.abs(cf[5:10])**0.5 + np.angle(cf[0:5])
        cf[20:25] = np.array([abs(t1 * t2)**(i+1) for i in range(5)])
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_63(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for i in range(25):
            numerator = (t1 * (i+1) + t2**((i+1)/2))
            denominator = np.abs(t1 * (i+1) + t2**(i+1))
            if denominator>0 :
                cf[i] =  numerator/denominator 
            else:
                cf[i] = 0
        cf[2] = t1.real + t2.imag
        cf[6] = np.abs(np.exp(1j * np.angle(t1 * t2)))
        cf[10] = (t1 * t2).real + (t1 / t2).imag
        cf[12] = np.angle(t1 + 4*t2) / np.abs(np.conj(t1 - 4*t2))
        cf[16] = np.abs(np.exp(1j * np.angle(t1 - t2)))
        cf[18] = (t1 / t2).real - (t1 * t2).imag
        cf[22] = np.abs(np.exp(1j * np.angle(t1 + t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_64(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = np.exp(1j * np.angle(t1 * np.conj(t2)))
        cf[2] = np.abs(t1) * np.abs(t2)
        for k in range(3, 25):
            cf[k] = (cf[k-1].real + 1j * cf[k-1].imag) * np.exp(1j * np.angle(cf[k-2]))
            if cf[k].imag == 0:
                cf[k] = cf[k] + 1e-10
            cf[k] = np.log(np.abs(cf[k])) / 2 + cf[k] * 1j
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_65(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1 - t2
        cf[2] = t1 * t2
        cf[3] = t1 / t2
        cf[4] = np.abs(t1) + np.abs(t2)
        cf[5] = np.abs(t1) - np.abs(t2)
        cf[6] = np.angle(t1) + np.angle(t2)
        cf[7] = np.angle(t1) - np.angle(t2)
        cf[8] = t1**2 + t2**2
        cf[9] = t1**3 + t2**3
        cf[10] = t1**4 + t2**4
        cf[11] = np.log(np.abs(t1)**2 + np.abs(t2)**2 + 1)
        cf[12] = np.exp(np.abs(t1) + np.abs(t2))
        cf[13] = np.conj(t1) * t2
        cf[14] = t1 * np.conj(t2)
        cf[15] = np.conj(t1) * np.conj(t2)
        cf[16] = np.abs(t1 - t2)
        cf[17] = np.abs(t1 + t2)
        cf[18:24] = np.abs(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_66(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1 - t2
        cf[2] = t1 * t2
        cf[3] = t1 / t2
        cf[4] = np.abs(t1) + np.abs(t2)
        cf[5] = np.abs(t1) - np.abs(t2)
        cf[6] = np.angle(t1) + np.angle(t2)
        cf[7] = np.angle(t1) - np.angle(t2)
        cf[8] = t1**2 + t2**2
        cf[9] = t1**3 + t2**3
        cf[10] = t1**4 + t2**4
        cf[11] = np.log(np.abs(t1)**2 + np.abs(t2)**2 + 1)
        cf[12] = np.exp(np.abs(t1) + np.abs(t2))
        cf[13] = np.conj(t1) * t2
        cf[14] = t1 * np.conj(t2)
        cf[15] = np.conj(t1) * np.conj(t2)
        cf[16] = np.abs(t1 - t2)
        cf[17] = np.abs(t1 + t2)
        cf[18:25] = np.abs(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_67(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1 * t2
        cf[2] = (t1 + t2)**2
        cf[3] = np.abs(t1) * np.abs(t2)
        for k in range(4, 9):
            r = (t1 + 1j*t2)**(k)
            cf[k] = r / np.abs(r)
        cf[9] = np.log(np.abs(t1)) / np.log(np.abs(t2))
        cf[10] = np.exp(np.angle(t1)) * np.exp(np.angle(t2))
        for k in range(11, 16):
            cf[k] = (np.conj(t1) / np.conj(t2)) * 1j**(k-2)
        cf[16] = t1.real * t2.imag
        cf[17] = t1.imag * t2.real
        for k in range(18, 23):
            z = (t1 + 1j*t2)**(k)
            cf[k] = np.sin(np.angle(z))
        cf[23] = np.cos(np.angle(t1) + np.angle(t2))
        cf[24] = np.tanh(np.abs(t1*t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_68(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            cf[k] = np.abs(t1)**((k+1)/2) * (np.cos((k+1) * np.angle(t2)) + 1j * np.sin((k+1) * np.angle(t2)))
        cf[4] = cf[4] + (np.log(np.abs(t1)) + np.log(np.abs(t2))) / 2
        cf[9] = cf[9] + np.conj(t1 * t2)
        cf[14] = cf[14] + np.abs(t2 - t1)**2
        cf[19] = cf[19] + (np.sin(np.angle(t1)) / np.cos(np.angle(t2)))**3
        cf[24] = cf[24] + ((1j * t1 - t2)**2 / (1 + np.abs(t1 + t2)**3))**4
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_69(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for i in range(25):
            cf[i] = (t1.real**(i+1) + t2.imag**(25-i))/(1 + np.abs(t1 + t2)) * np.exp(1j * np.angle(t1 + t2))
        cf[2] = 3 * np.conj(t1**2 + t2)
        cf[6] = 7 * np.abs(t1 + t2)
        cf[10] = 11 * (t1/t2 + np.conj(t2/t1))
        cf[16] = 17 * (np.abs(t1)*abs(t2))/(np.abs(t1 + t2))**2
        cf[22] = 23 * (np.conj(t1) + t2) / (1 + np.abs(t1 * np.conj(t2)))
        cf[24] = 25 * (np.conj(t1) + np.conj(t2)) / np.abs(t1*t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)
    
def poly_giga_7(t1, t2):
    try:
        pi  =  np.pi
        n   =  30
        rec =  np.linspace(np.real(t1), np.real(t2), n)
        imc =  np.linspace(np.imag(t1), np.imag(t2), n)
        f1  =  np.exp(1j * np.sin(10 * pi * imc))
        f2  =  np.exp(1j * np.cos(10 * pi * rec))
        f   =  f1 + f2
        return  f
    except:
        return np.zeros(30, dtype=np.complex128)

def poly_giga_7a(t1, t2):
    try:
        pi  =  np.pi
        n   =  30
        tt1 =  np.exp(1j * 2 * pi * t1)
        tt2 =  np.exp(1j * 2 * pi * t2) 
        rec =  np.linspace(np.real(tt1), np.real(tt2), n)
        imc =  np.linspace(np.imag(tt1), np.imag(tt2), n)
        f1  =  np.exp(1j * np.sin(10 * pi * imc))
        f2  =  np.exp(1j * np.cos(10 * pi * rec))
        f   =  f1 + f2
        return f
    except:
        return np.zeros(30, dtype=np.complex128)
    
def poly_giga_7b(t1, t2):
    try:
        pi  =  np.pi
        n   =  23
        tt1 =  np.exp(1j * 2 * pi * t1)
        ttt1 =  np.exp(1j * 2 * pi * np.real(tt1))
        tttt1 =  np.exp(1j * 2 * pi * np.imag(ttt1))
        tt2 =  np.exp(1j * 2 * pi * t2)
        ttt2 =  np.exp(1j * 2 * pi * np.imag(tt2))
        tttt2 =  np.exp(1j * 2 * pi * np.real(ttt2)) 
        rec =  np.linspace(np.real(tt1), np.real(tttt2), n)
        imc =  np.linspace(np.imag(tt2), np.imag(tttt1), n)
        f1  =  np.exp(1j * np.sin(10 * pi * imc))
        f2  =  np.exp(1j * np.cos(10 * pi * rec))
        f   =  f1 + f2
        return f
    except:
        return np.zeros(23, dtype=np.complex128)

def poly_giga_7c(t1, t2):
    try:
        pi   =  np.pi
        n    =  23
        tt1  =  np.exp(1j * 2 * pi * t1)
        ttt1 =  np.exp(1j * 2 * pi * tt1)
        rec  =  np.linspace(np.real(tt1), np.real(ttt1), n)
        imc  =  np.linspace(np.imag(tt1), np.imag(ttt1), n)
        f1   =  np.exp(1j * np.sin(11 * pi * imc))
        f2   =  np.exp(1j * np.cos(13 * pi * rec))
        f    =  f1 + f2
        return f
    except:
        return np.zeros(23, dtype=np.complex128)

def p7c0(t1, t2):
    try:
        pi   =  np.pi
        n    =  23
        tt1  =  np.exp(1j * 2 * pi * t1)
        ttt1 =  np.exp(1j * 2 * pi * tt1)
        rec  =  np.linspace(np.real(tt1), np.real(ttt1), n)
        imc  =  np.linspace(np.imag(tt1), np.imag(ttt1), n)
        f1   =  np.exp(1j * np.sin(11 * pi * imc))
        f2   =  np.exp(1j * np.cos(13 * pi * rec))
        f    =  f1 + f2
        f[0] = f[0] + 0*np.exp(1j * 2 * pi * t2)
        return f
    except:
        return np.zeros(23, dtype=np.complex128)

def p7c1(t1, t2):
    try:
        pi   =  np.pi
        n    =  23
        tt1  =  np.exp(1j * 2 * pi * t1)
        ttt1 =  np.exp(1j * 2 * pi * tt1)
        rec  =  np.linspace(np.real(tt1), np.real(ttt1), n)
        imc  =  np.linspace(np.imag(tt1), np.imag(ttt1), n)
        f1   =  np.exp(1j * np.sin(11 * pi * imc))
        f2   =  np.exp(1j * np.cos(13 * pi * rec))
        f    =  f1 + f2
        f[0] = f[0] + 0.1*np.exp(1j * 2 * pi * t2)
        return f
    except:
        return np.zeros(23, dtype=np.complex128)

def p7ct2(t1, t2):
    try:
        pi   =  np.pi
        n    =  23
        tt1  =  np.exp(1j * 2 * pi * t1)
        ttt1 =  np.exp(1j * 2 * pi * tt1)
        rec  =  np.linspace(np.real(tt1), np.real(ttt1), n)
        imc  =  np.linspace(np.imag(tt1), np.imag(ttt1), n)
        f1   =  np.exp(1j * np.sin(11 * pi * imc))
        f2   =  np.exp(1j * np.cos(13 * pi * rec))
        f    =  f1 + f2
        f[0] = f[0] + 0.1 * t1 * np.exp(1j * 2 * pi * t2)
        return f
    except:
        return np.zeros(23, dtype=np.complex128)        

def p7cm1(t1, t2):
    try:
        pi2    =  2 * np.pi
        n      =  21
        v      =  np.linspace(t1, np.cos(409 * pi2 * t1), n)
        f      =  np.exp(1j * np.sin( 613 * pi2 * v))
        f[n//2-1] +=  1j * 11 * t2 * np.exp( 1j * (1/5) * pi2 * t2)
        f[n//2+1] +=  1j * 13 * t2 * np.exp( 1j * (1/7) * pi2 * t2)
        return f
    except:
        return np.zeros(21, dtype=np.complex128)     
    

def poly_giga_7d(t1, t2):
    try:
        pi   =  np.pi
        n    =  61
        tt1  =  np.exp(1j * 2 * pi * t1)
        ttt1 =  np.exp(1j * 2 * pi * np.real(tt1))
        tttt1 =  np.exp(1j * 2 * pi * np.real(ttt1))
        rec  =  np.linspace(np.real(tt1), np.real(ttt1), n)
        imc  =  np.linspace(np.imag(ttt1), np.imag(tttt1), n)
        f1   =  np.exp(1j * np.sin(11 * pi * imc))
        f2   =  np.exp(1j * np.sin(67 * pi * rec))
        f    =  f1 + f2
        f[0] = f[0] + np.exp(1j * 2 * pi * t2)
        return f
    except:
        return np.zeros(61, dtype=np.complex128)
       
def p7e(t1, t2):
    try:
        pi  =  np.pi
        n   =  30
        rec =  np.linspace(np.real(t1), np.real(t2), n)
        imc =  np.linspace(np.imag(t1), np.imag(t2), n)
        f1  =  np.exp(1j * np.sin(10 * pi * imc))
        f2  =  np.exp(1j * np.cos(10 * pi * rec))
        f   =  f1 + f2
        return  np.roots(f)
    except:
        return np.zeros(30, dtype=np.complex128)

def p7f(t1, t2):
    try:
        pi2  =  2 * np.pi
        n    =  23
        tt1  =  np.exp(1j * 2 * pi * t1)
        ttt1 =  np.exp(1j * 2 * pi * tt1)
        v  =  np.linspace(np.real(tt1), np.real(ttt1), n)
        if t2 < 0.1:
            f = 10 * t1 * np.exp(1j * np.sin(11 * pi2 * v))
        elif 0.1 <= t2 < 0.2:
            f =  100 * np.exp(1j * np.sin(17 * pi2 * v))
        elif 0.2 <= t2 < 0.3:
            f =  599 * np.exp(1j * np.cos(83 * pi2 * v))
        elif 0.3 <= t2 < 0.4:
            f =  443 * np.exp(1j * np.sin(179 * pi2 * v))
        elif 0.4 <= t2 < 0.5:
            f =  293 * np.exp(1j * np.sin(127 * pi2 * v))
        elif 0.5 <= t2 < 0.6:
            f =  541 * np.exp(1j * np.sin(103 * pi2 * v))
        elif 0.6 <= t2 < 0.7:
            f =  379 * np.exp(1j * np.sin(283 * pi2 * v))
        elif 0.7 <= t2 < 0.8:
            f =  233 * np.exp(1j * np.sin(3 * pi2 * v))
        elif 0.8 <= t2 < 0.9:
            f =  173 * np.exp(1j * np.sin(5 * pi2 * v))
        else:
            f =  257 * np.exp(1j * np.sin(23 * pi2 * v))

        f[n-1] +=  211 * np.exp(1j * pi2 * (1/7) * t2 )

        return f

    except:
        return np.zeros(23, dtype=np.complex128)    


def p8f(t1, t2):
    n = 23
    r = 0.5 + np.sign( t1 - 0.5 ) * 0.25
    i = 0.5 + np.sign( t2 - 0.5 ) * 0.25
    tt1 = 2 * np.abs( t1 - 0.5 )
    tt2 = 2 * np.abs( t2 - 0.5 )
    v= np.linspace( tt1, tt2, n )
    uc = 0.25 * tt1 * tt2 * np.exp( 1j * 2 * np.pi * v )
    cf = r + 1j*i + uc
    return cf.astype(np.complex128)

def p9f(t1, t2):
    n   = 23
    r   = 0.5 * ( np.sign( t1 - 0.5 ) + 1 )
    i   = 0.5 * ( np.sign( t2 - 0.5 ) + 1 )
    z   = r + 1j * i
    tt1 = np.abs( t1 - 0.5 ) 
    tt2 = np.abs( 0.5 - t2 ) 
    v   = np.linspace( 0, 0.5 , n //2 )
    v1 =  v + 1j * tt1
    v2 =  v * 1j + tt2
    cf =  z  + v2 + v1
    return cf.astype(np.complex128)

def p10f(t1, t2):
    n = 5
    v = np.linspace( 1/n, 1-2/n , n )
    rp, ip = np.meshgrid(v, v, indexing='ij')
    c0  =  rp + 1j * ip
    c1  =  c0.flatten()
    z0  = np.floor(t1*10) +  np.floor(t2*10) * 1j
    z1  = (t1*10 - np.floor(t1*10)) + (t2*10 - np.floor(t2*10)) * 1j 
    cf = z0 + (1/n) * z1 + c1
    return cf.astype(np.complex128)


def poly_giga_70(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0:5] = np.real(t1) * np.arange(1, 6) - np.imag(t2) * np.arange(1, 6)
        cf[5] = np.abs(t1) * np.abs(t2)
        cf[6:11] = np.angle(t1+t2) * np.arange(6, 11)
        cf[11] = np.conj(t1) + np.conj(t2)
        cf[12:17] = np.real(t1 + 1j * t2) * np.arange(1, 6)
        cf[17] = np.angle(t1) * np.angle(t2)
        cf[18:23] = np.imag(t1 - 1j * t2) * np.arange(1, 6)
        cf[23] = np.conj(t1 * t2)
        cf[24] = np.abs(cf[11]) + np.angle(cf[17])
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_71(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        for i in range(1, 25):
            cf[i] = np.real(t1)*np.imag(t2)*(np.abs(t1)*np.angle(t2))**(i+1) / (np.conj(t1)*np.conj(t2))**i
            if np.isinf(np.abs(cf[i])) or np.isnan(cf[i]):
                cf[i] = 0
        cf[4] = cf[4] + np.log(np.abs(cf[2]))*cf[1]
        cf[9] = cf[9] + cf[0]*np.conj(cf[3])
        cf[14] = cf[14] + cf[1]*cf[2]
        cf[19] = cf[19] + cf[3]*cf[0]
        cf[24] = cf[24] + cf[4]
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_72(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, 25):
            v = np.sin(k * cf[k-1] + np.angle(t2**k)) + np.cos(k * np.abs(t1))
            cf[k] = v / (np.abs(v) + 1e-10)
        cf[9] = t1 * t2 - np.abs(t2)**2 + 1j * np.angle(t1)
        cf[14] = np.conj(t1)**3 - np.angle(t2)**3 + 1j * np.abs(t2)
        cf[19] = np.abs(t2)**3 + t1**2 + t2**2 + 1j * np.angle(t2)**2
        cf[24] = np.abs(t1 * t2) + np.angle(t1)**5 + 1j * np.abs(t1)**5
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_73(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1 - t2
        cf[2] = t1*t2
        cf[3] = t1/t2
        cf[4] = (t1 + t2)**2
        cf[5] = (t1 - t2)**2
        cf[6] = (t1**2 + t2**2)
        cf[7] = (t1**2 - t2**2)
        cf[8] = (t1**2 + t2**2)**2
        cf[9] = (t1**2 - t2**2)**2
        cf[10] = (t1 + t2 + 1j)**2
        cf[11] = (t1 - t2 - 1j)**2
        cf[12] = (t1 + 1j*t2)**3
        cf[13] = (1j*t1 - t2)**3
        cf[14] = (t1 + t2)**3 + (t1 - t2)**3
        cf[15] = (t1*t2)**3 - 1j*t1*t2
        cf[16] = (t1/t2)**4 + 1j*t1/t2
        cf[17] = (t1*t2 + 1j)**4 - t1*t2
        cf[18] = (t1 + t2 + 1j)**5 - (t1 + t2)
        cf[19] = (t1 - t2 - 1j)**5 + (t1 - t2)
        cf[20] = (t1 + 1j*t2)**6 - 1j*t1*t2
        cf[21] = (1j*t1 - t2)**6 + t1*t2
        cf[22] = (t1 + t2)**7 - (t1 - t2)**7
        cf[23] = (t1*t2)**8 - (t1/t2)**8
        cf[24] = np.log(np.abs(t1 + 1j*t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_74(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1*t2
        cf[1] = np.real(t1) + 2*np.imag(t2)
        cf[2] = t1/np.abs(t2)
        if np.abs(t2) != 0:
            cf[3] = t2/np.abs(t1)
        cf[4] = np.angle(t1)*np.angle(t2)
        cf[5] = np.abs(np.conj(t1)*t2)
        cf[6] = np.abs(t1-t2)
        cf[7] = np.angle(t1*t2)+np.conj(np.angle(t1*t2))
        cf[8] = np.log(np.abs(t1)) + np.log(np.abs(t2))
        cf[9] = np.abs(t1)*np.imag(t2)
        cf[14] = np.abs(t1)**2 + np.abs(t2)**2
        cf[19] = np.real(t1)**3 + np.imag(t2)**2
        cf[24] = 1j*(np.abs(t1) + np.abs(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_75(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.abs(t1) * np.abs(t2)
        cf[1] = np.abs(t1 + t2)
        cf[2] = np.abs(np.conj(t1) + np.conj(t2))
        cf[3] = np.angle(t1) * np.angle(t2)
        cf[4] = np.angle(np.conj(t1) + np.conj(t2))
        cf[5] = np.exp(1j * np.angle(t1 + t2))
        cf[6] = np.exp(1j * np.angle(np.conj(t1) + np.conj(t2)))
        cf[7] = np.sin(np.abs(np.conj(t1)) * np.abs(np.conj(t2)))
        cf[8] = np.cos(np.angle(t1 + t2))
        cf[9] = np.tanh(np.abs(np.conj(t1 + t2)))
        cf[10:15] = np.abs(t1)**((np.arange(11,16)/10)) * np.abs(t2)**((np.arange(15,10,-1)/10))
        cf[15:20] = (np.arange(16,21)) * (np.angle(t1) + np.angle(t2)) / 2
        cf[20:25] = np.real(t1)**2 + np.imag(t2)**2 + np.arange(21,26)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_76(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, 25):
            v = (t1+t2)**(k+1) + np.sin(k * cf[k-1]) + np.log(np.abs(k * t1)) - np.log(np.abs((k+1) * t2))
            cf[k] = v / np.abs(v)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_77(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0:10] = np.arange(1, 11) * t1 + np.arange(11, 21) * 1j * t2
        cf[10:20] = (t1 + 1j * t2)**2 * np.arange(11, 21)
        cf[20:25] = (np.abs(t1) + np.angle(t2)) * np.arange(1, 6)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_78(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            cf[k] = (k + 1 + t1) / (k + 1 + t2)
        cf[4] = cf[4] + np.log(np.abs(t1 + t2))
        cf[9] = cf[9] + np.sin(np.real(t1)) + np.cos(np.imag(t2))
        cf[14] = cf[14] + np.abs(cf[13])**2 + np.angle(cf[12])**2
        cf[19] = cf[19] + np.abs(np.real(t2) * np.imag(t1))
        cf[24] = cf[24] + np.abs(t1 + np.conj(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_79(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0:10] = np.arange(1, 11) * t1 + np.arange(11, 21) * 1j * t2
        cf[10:20] = (t1 + 1j * t2)**2 * np.arange(11, 21)
        cf[20:25] = (np.abs(t1) + np.angle(t2)) * np.arange(1, 6)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_8(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        roots1 = np.roots([1, t1**3, -50 * t2, 100 * t1, 10j])
        roots2 = np.roots([1, roots1[0], -np.real(roots1[1]), np.imag(roots1[2])])
        cf[0:4] = roots1
        cf[9:13] = roots2
        cf[19] = 50 * t1 * t2 + np.real(roots2[0])
        cf[29] = np.exp(1j * t1) + 50 * t2**3
        cf[34] = 200 * np.exp(1j * t1**3) - np.exp(-1j * t2**2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_80(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            cf[k] = (k + 1 + t1) / (k + 1 + t2)
        cf[4] = cf[4] + np.log(np.abs(t1 + t2))
        cf[9] = cf[9] + np.sin(np.real(t1)) + np.cos(np.imag(t2))
        cf[14] = cf[14] + np.abs(cf[13])**2 + np.angle(cf[12])**2
        cf[19] = cf[19] + np.abs(np.real(t2) * np.imag(t1))
        cf[24] = cf[24] + np.abs(t1 + np.conj(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_81(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for i in range(25):
            cf[i] = (t1 * (i+1) + t2**((i+1)/2)) / np.abs(t1 * (i+1) + t2**(i+1))
        cf[2] = np.real(t1) + np.imag(t2)
        cf[6] = np.abs(np.exp(1j * np.angle(t1 * t2)))
        cf[10] = np.real(t1 * t2) + np.imag(t1 / t2)
        cf[12] = np.angle(t1 + 4*t2) / np.abs(np.conj(t1 - 4*t2))
        cf[16] = np.abs(np.exp(1j * np.angle(t1 - t2)))
        cf[18] = np.real(t1 / t2) - np.imag(t1 * t2)
        cf[22] = np.abs(np.exp(1j * np.angle(t1 + t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_82(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, 25):
            cf[k] = (1j * cf[k-1]**2 + 2j * t1) / (t2 + 1)
            if np.abs(cf[k]) == 0:
                cf[k] = 1
            if np.isinf(np.abs(cf[k])):
                cf[k] = 1
        
        cf[4] = cf[4] + np.sin(cf[1].real) + np.log(np.abs(t1))
        cf[9] = cf[9] + np.cos(cf[4].imag) + np.log(np.abs(t2))
        cf[14] = cf[14] + np.tan(cf[9].real) + np.log(np.abs(t1 * t2))
        cf[19] = cf[19] + np.arctan(cf[14].imag) + np.log(np.abs(np.conj(t1) * t2))
        cf[24] = cf[24] + np.sin(np.angle(cf[19])) + np.log(np.abs(np.conj(t1) * np.conj(t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_83(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1.real * t2.real + t1.imag * t2.imag
        cf[1] = np.abs(t1) * np.abs(t2)
        cf[2] = np.angle(t1) + np.angle(t2)
        cf[3] = np.conj(t1).real + np.conj(t2).imag
        
        for k in range(4, 25):
            cf[k] = (cf[k-1] * cf[k-4] + cf[k-3] * cf[k-2]) / np.abs(cf[k-1] * cf[k-4] + cf[k-3] * cf[k-2])
        
        cf[12] = cf[0] / cf[1] + cf[2] / cf[3]
        cf[18] = np.log(np.abs(cf[0] * cf[1])+1) / np.log(np.abs(cf[2] * cf[3])+1)
        cf[24] = cf[4] * cf[9] * cf[14] * cf[19] / np.abs(cf[4] * cf[9] * cf[14] * cf[19])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_84(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17])
        cf[0:7] = primes * (t1 + t2)
        cf[14] = np.sum(primes[0:5]) + (t1+1j*t2)**2
        cf[24] = np.prod(primes[0:4]) / np.abs(t1 + 1j * t2)
        
        for k in range(7, 14):
            try:
                v = np.log(np.abs(t1)) * np.sin(cf[k-1]) + np.log(np.abs(t2)) * np.cos(cf[k-1])
                if not np.isinf(v):
                    cf[k] = v
            except:
                pass
        cf[15:24] = np.real(t1) + np.imag(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_85(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        
        for k in range(25):
            cf[k] = np.sin(t1)+(1j*np.cos(t2.real**2-t2.imag**2))/(np.sqrt(np.abs(t1)**2+abs(t2)**2))
        
        cf[2] = cf[6]*cf[10]*cf[18]/cf[22]
        cf[4] = cf[9] + cf[14] + cf[19] - cf[24]
        cf[8] = 1j*t1*t2*(t1 - t2)
        cf[12] = cf[4]*t1/(1+abs(t2))
        cf[16] = np.conj(cf[8])/t2
        cf[20] = np.log(np.abs(cf[4]*t2/(1+abs(t1))))
        return cf.astype(np.complex128)
    except:
        return np.zeros(25, dtype=np.complex128)

def poly_giga_86(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = (t1**3).real*(t2**3).real
        cf[1] = (t1**3).imag*(t2**3).imag
        
        for k in range(2, 25):
            if k % 3 == 0:
                cf[k] = (t1+1j*t2)**(k/3) / k
            else:
                cf[k] = np.conj(cf[k-1]) ** 2 + np.abs(t1)*abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_87(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = 1 + t1*t2 + np.log(np.abs(t1 + t2) + 1)
        cf[2] = t1 + t2 + np.log(np.abs(1 - t1 * t2) + 1)
        
        for i in range(3, 51):
            cf[i] = i * t1 + (51 - i) * t2 + np.log(np.abs(t1 - t2 * i) + 1)
        
        cf[10] = cf[0] + cf[9] - np.sin(t1)
        cf[20] = cf[30] + cf[40] - np.cos(t2)
        cf[30] = cf[20] + cf[40] + np.sin(t1)
        cf[40] = cf[30] + cf[20] - np.cos(t2)
        cf[50] = cf[40] + cf[20] + np.sin(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_88(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71])
        cf[0:20] = (t1**primes[0:20] + t2**(primes[0:20] + 1)) / np.abs(t1 - t2)
        
        for k in range(20, 51):
            cf[k] = (np.cos(k * np.log(np.abs(t1) + 1)) + np.sin(k * np.log(np.abs(t2) + 1))) / k
        
        cf[50] = np.abs(t1) * np.abs(t2) * (np.cos(np.angle(t1) * np.angle(t2)) - 1j * np.sin(np.angle(t1) * np.angle(t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_89(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        for i in range(51):
            cf[i] = np.cos(i*t1) + np.sin(i*t2)
        
        cf[0] = cf[0]*t1**50
        cf[1] = cf[1]*t2**49
        
        for i in range(2, 51):
            cf[i] = cf[i]*t1**(51-1j)*t2**(i-2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_9(t1, t2):
    try:
        n = 20
        re1 = t1.real
        im1 = t1.imag
        re2 = t2.real
        im2 = t2.imag
        rec = np.linspace(re1, re2, n)
        imc = np.linspace(im1, im2, n)
        cf = 100j * imc**9 + 100 * rec**9
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_90(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = t1 + t2
        for i in range(1, 51):
            cf[i] = ((np.cos(t1)/np.sin(t2))**i + (np.sin(t1)/np.cos(t2))**(2j)) * cmath.phase(t1 + t2)**i
        cf[1:51] = np.log(np.abs(cf[1:51]) + 1) / np.log(i+1)
        cf[4] = cf[4] * np.abs(t1 + t2)
        cf[9] = cf[9] * (t1 * t2.conjugate()).real
        cf[19] = cf[19] * (t2 * t1.conjugate()).imag
        cf[29] = cf[29] * cmath.phase(t1 + t2)
        cf[39] = cf[39] * np.abs(t1 + t2)
        cf[49] = cf[49] * (t1 * t2.conjugate()).real
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_91(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, 71):
            r = np.abs(t1)**k + np.abs(t2)**(71-k)
            theta = cmath.phase(t1)**k - cmath.phase(t2)**(71-k)
            cf[k] = r * np.cos(theta) + r * np.sin(theta)*1j
        cf[2:70] = cf[2:70] + np.log(np.abs(t2-t1)+1)
        cf[70] = cf[70] + (t1*t2).conjugate()
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_92(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(71):
            cf[i] = (np.sin(t1)*np.cos(t2)**i) + (np.cos(t1)*np.sin(t2)**i) + np.log(np.abs(t1+t2)+1)**i
        cf[0] = cf[0] + t1.real*t2.imag + t1.imag*t2.real
        cf[1] = cf[1] + np.abs(t1) * np.abs(t2)
        cf[2] = cf[2] + cmath.phase(t1) / cmath.phase(t2)
        cf[3] = cf[3] + cmath.phase(t2) / cmath.phase(t1)
        cf[4] = cf[4] + t1.real / np.abs(t2)
        cf[5] = cf[5] + t2.imag / np.abs(t1)
        cf[70] = cf[70] + (t1+t2).conjugate()
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def giga_92_old(t1, t2):
    # Create an array of 71 complex numbers (initialized to 0)
    cf = np.zeros(71, dtype=np.complex128)
    
    # Loop over i from 1 to 71 (adjusting index with i-1 for Python)
    for i in range(1, 72):
        term1 = np.sin(t1) * (np.cos(t2) ** i)
        term2 = np.cos(t1) * (np.sin(t2) ** i)
        term3 = np.log(np.abs(t1 + t2) + 1) ** i
        cf[i - 1] = term1 + term2 + term3

    # Adjust specific elements (adjusting indices: cf[1] in R is cf[0] in Python, etc.)
    cf[0] += np.real(t1) * np.imag(t2) + np.imag(t1) * np.real(t2)
    cf[1] += np.abs(t1) * np.abs(t2)
    cf[2] += np.angle(t1) / np.angle(t2)
    cf[3] += np.angle(t2) / np.angle(t1)
    cf[4] += np.real(t1) / np.abs(t2)
    cf[5] += np.imag(t2) / np.abs(t1)
    cf[70] += np.conjugate(t1 + t2)
    
    return cf.astype(np.complex128)

def poly_giga_93(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        angles = np.linspace(0, 2*np.pi, 35)
        for i in range(35):
            cf[i] = np.cos(angles[i] * t1) + np.sin(angles[i] * t2) / np.abs(t1)
        for i in range(35, 71):
            cf[i] = (t1*1j + t2*(71-1j))**3 / (t1*1j + 1j*t2*(71-1j))**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_94(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1.real * t2.imag
        for k in range(1, 71):
            cf[k] = np.sin(k * cf[k-1]) + np.cos(k * t1)
            cf[k] = cf[k] / np.abs(cf[k])
        cf[30] = np.abs(cf[14])**2 + cmath.phase(t2)**2
        cf[40] = cf[20] * (np.abs(t1) + np.log(np.abs(t2)+1))
        cf[50] = cf[30] + np.log(np.abs(t1*t2)+1)
        cf[60] = np.abs(t1 + t2) * cmath.phase(cf[30])
        cf[70] = np.abs(cf[34]) / (t1 + 1j * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def giga_94_old(t1, t2):
    cf = np.zeros(71, dtype=np.complex128)
    cf[0] = t1.real * t2.imag
    for k in range(1, 71):
        factor = k + 1
        cf[k] = np.sin(factor * cf[k-1]) + np.cos(factor * t1)
        cf[k] = cf[k] / np.abs(cf[k])  # Normalize by its modulus
    cf[30] = np.abs(cf[14])**2 + np.angle(t2)**2
    cf[40] = cf[20] * (np.abs(t1) + np.log(np.abs(t2) + 1))
    cf[50] = cf[30] + np.log(np.abs(t1 * t2) + 1)
    cf[60] = np.abs(t1 + t2) * np.angle(cf[30])
    cf[70] = np.abs(cf[34]) / (t1 + 1j * t2)
    return cf.astype(np.complex128)

def poly_giga_95(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1.real * t2.imag
        cf[1] = np.abs(t1) / cmath.phase(t2)
        cf[2] = np.sin(t1) * np.cos(t2)
        for i in range(3, 71):
            cf[i] = cf[i-1] + cf[i-3] * np.log(np.abs(cf[i-2]+1))
            if i % 2 == 0:
                cf[i] = cf[i] + np.abs(cf[i-1] * t1)
            else:
                cf[i] = cf[i] - np.abs(cf[i-1] * t2.conjugate())
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_96(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(71):
            cf[i] = ((i+1)**2 + 3j + 1)*(t1**2) + ((i+1)**3 - (i+1)**2 + 1)*(t2**2) + np.sin(t1*1j + t2) + np.log(np.abs(t1*1j - t2)+1)
            if (i+1) % 2 == 0:
                cf[i] = cf[i] + (t1+1j*t2)**2
            elif (i+1) % 3 == 0:
                cf[i] = cf[i] + np.abs(t1+1j*t2)**3
            else:
                cf[i] = cf[i] + (t1+1j*t2).real**4
        cf[0] *= 10000
        cf[1] *= 1000
        cf[2] *= 100
        cf[3] *= 10
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_97(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 * t2 * np.abs(t1) * np.abs(t2) + np.log(np.abs(t1)+1) + np.log(np.abs(t2)+1)
        cf[1:3] = (t1 + t2) * np.array([1, -1])
        for i in range(3, 10):
            cf[i] = np.sin(i*t1*t2) / ((i/t1/t2).real)
        for i in range(10, 20):
            cf[i] = np.cos(i*t1*t2) / ((i/t1/t2).imag)
        for i in range(20, 30):
            cf[i] = np.log(np.abs(t1**i+t2**i) + 1)
        cf[30:40] = (t1 + t2) ** np.arange(1, 11)
        for i in range(40, 50):
            cf[i] = np.abs(i/t1) * np.abs(i/t2)
        cf[50:60] = sum(np.prod(cf[:30] * np.array([-1, 1])))
        for i in range(60, 70):
            cf[i] = np.abs(t1)**i - np.abs(t2)**i + np.log(np.abs(t1**i+t2**i) + 1) + cmath.phase(cf[i-10])
        cf[70] = np.sum(cf[:70]).conjugate()
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_98(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        for i in range(1, 71):
            cf[i] = np.sin(i * cmath.phase(t1 * t2)) * np.abs(cf[i-1]**(i-1)) * np.cos(np.abs(t1) + np.abs(t2))
        for i in range(34, 55):
            cf[i] = cf[i] * np.abs(t1 + t2) * cf[i-1]
        cf[4:26] = cf[4:26] * cf[0] * (np.abs(t1) * np.abs(t2))**np.arange(1, 23)
        cf[55:71] = cf[55:71] * cf[0] / (np.abs(t1) * np.abs(t2))**np.arange(15, 0, -1)
        indices = [0, 14, 29, 44, 59, 70]
        cf[indices] = cf[indices] + (t1 * t2).conjugate()
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_giga_99(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        complex_val = np.sin(t1 + t2) + np.cos(t1 - t2)
        for k in range(71):
            if (k+1) % 2 == 0:
                cf[k] = complex_val / np.abs(k+1) - np.abs(t1)
            else:
                cf[k] = complex_val * np.abs(k+1) + np.log(np.abs(k+1) + 1) + t2.imag - t1.real
            if (k+1) % 3 == 0 and k > 0:
                cf[k] = cf[k] + 3 * cf[k-1]
            if (k+1) % 5 == 0 and k > 1:
                cf[k] = cf[k] + 5 * cf[k-2]
            if (k+1) % 7 == 0 and k > 2:
                cf[k] = cf[k] + 7 * cf[k-3]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_1(t1, t2):
    try:
        cf = np.zeros(36, dtype=np.complex128)
        for i in range(1, 37):
            cf[i-1] = np.sin(t1**(i/2)) * np.cos(t2**(i/3)) + (i**2) * t1 * t2 + np.log(np.abs(t1 + t2) + 1) * 1j * i
        cf[10] = t1 * t2 * np.real(cf[6]) + np.imag(cf[18]) * t1**3
        cf[21] = t2 * cf[10] + np.real(cf[34]) * t1**3
        cf[32] = cf[21] - np.real(cf[16]) * t1**2
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(36, dtype=np.complex128)

def clip_complex(z, mag_limit=1e6):
    """
    If the magnitude of the complex number z exceeds mag_limit,
    scale it back to have magnitude mag_limit (preserving its phase).
    """
    r = np.abs(z)
    if r > mag_limit:
        return z * (mag_limit / r)
    return z

def poly_1a(t1, t2, mag_limit=1e6):
    """
    Computes an array of 36 complex coefficients from two complex numbers t1 and t2.
    
    Some of the terms involve np.sin and np.log which, when given complex inputs,
    can return very large values. To avoid numerical overflows we clip the outputs
    of these functions using a helper (clip_complex). You can adjust mag_limit as needed.
    
    Parameters:
      t1, t2: complex numbers (e.g., of magnitude ~1)
      mag_limit: maximum allowed magnitude for intermediate outputs
      
    Returns:
      A numpy array of shape (36,) with dtype complex128.
    """
    cf = np.zeros(36, dtype=np.complex128)
    
    for i in range(1, 37):
        # Compute terms using the given formula.
        # (Note: t1**(i/2) and t2**(i/3) are computed directly, since their exponents
        # are moderate. The problem arises in the unbounded growth of sin, cos, and log.)
        arg1 = t1**(i/2)
        arg2 = t2**(i/3)
        term_sin = np.sin(arg1)
        term_cos = np.cos(arg2)
        term_prod = (i**2) * t1 * t2
        term_log = np.log(np.abs(t1 + t2) + 1) * 1j * i

        # Clip each function’s output to avoid runaway (overflow) values.
        term_sin = clip_complex(term_sin, mag_limit)
        term_cos = clip_complex(term_cos, mag_limit)
        term_prod = clip_complex(term_prod, mag_limit)
        term_log = clip_complex(term_log, mag_limit)
        
        # Compute the combined coefficient.
        cf[i - 1] = clip_complex(term_sin * term_cos, mag_limit) + term_prod + term_log

    # The remaining adjustments don’t involve sin or log, but you can clip if needed.
    cf[10] = t1 * t2 * np.real(cf[6]) + np.imag(cf[18]) * (t1**3)
    cf[21] = t2 * cf[10] + np.real(cf[34]) * (t1**3)
    cf[32] = cf[21] - np.real(cf[16]) * (t1**2)
    
    return cf

def poly_2(t1, t2):
    try:
        cf = np.zeros(36, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(2, 37):
            v = np.sin(k * cf[k-2]) + np.cos(k * t1) + np.real(k * t2) * np.imag(k * cf[k-2])
            cf[k-1] = v / np.abs(v)
        cf[17] = t1**2 + np.real(t1) * t2 - np.imag(t2**2)
        cf[31] = 2 * (t1 + t2) - np.real(t1 * t2) + np.sin(np.real(t1)) * np.cos(np.imag(t2))
        cf[35] = cf[17] * cf[31] + np.sin(np.real(t1 * t2)) - np.cos(np.imag(t1 * t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_3(t1, t2):
    try:
        cf = np.zeros(36, dtype=np.complex128)
        for k in range(2, 37):
            v = (t1 ** k) / 2 + np.cos(t2 ** (k - 1)) + np.log(np.abs(t1) + 1)
            if k % 2 == 0:
                cf[k-1] = np.real(v)
            else:
                cf[k-1] = np.imag(v)
        cf[0] = 1 + t1 * t2
        cf[17] = cf[2] * cf[4] - cf[3] * cf[1]
        cf[23] = cf[11] / cf[5] + cf[7] * cf[9]
        cf[35] = np.sum(cf[15:20])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_4(t1, t2):
    try:
        cf = np.zeros(36, dtype=np.complex128)
        for k in range(1, 37):
            cf[k-1] = (t1 ** (36 - k) + t2 ** (36 - k)) / (k * 1j)
        cf[16] = t1 * t2 + np.log(np.abs(t1) + 1) - np.sin(t2)
        cf[24] = np.real(t1) - np.imag(t1) + 1j * (np.real(t2) + np.imag(t2))
        cf[29] = np.abs(t1)**2 - np.abs(t2)**2 + 1j * np.angle(t1) * np.angle(t2)
        cf[35] = np.conj(t1 * t2)**2 - np.sin(t1 + t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_5(t1, t2):
    try:
        cf = np.zeros(36, dtype=np.complex128)
        p = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53])
        for k in range(1, 17):
            cf[k-1] = np.sin(p[k-1] * t1) + np.cos(p[k-1] * t2)
        for k in range(17, 33):
            cf[k-1] = np.log(np.abs(p[k-17] * t1 + t2)) / (t1 + t2)
        cf[32] = np.prod(p[0:4]) / (t1 * t2)
        cf[33] = np.sum(p[4:8]) - t1**2 + t2**2
        cf[34] = p[8] * p[9] * (t1 + t2)
        cf[35] = p[10] * p[11] / (t1 - t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_6(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        for k in range(1, 52):
            cf[k-1] = (t1 + t2) * np.sin(np.log(np.abs(t1 * t2)**k + 1)) + np.cos(np.angle(t1 * t2)**k) * np.conj(t1 - t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_7(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        for k in range(1, 52):
            cf[k-1] = (
                np.cos(t1 * k)
                + 1j * np.sin(t2 * k)
                + np.log(np.abs(t1) + 1)
                + np.log(np.abs(t2) + 1)
            )
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47])
        
        # Fix the slice here:
        cf[1:16] += primes * (np.real(t1) + np.imag(t2)**2)

        cf[24] += np.sum(primes[0:5]) / (t1 + t2)
        cf[49] *= (
            np.real(t1 * t2)
            + np.imag(t1 * t2)
            + np.log(np.abs(t1 * t2) + 1)
        )
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_8(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0:25] = np.arange(1, 26) * (t1**2 + 1j * t2**3)
        cf[25] = np.abs(t1 + t2)
        cf[26:51] = np.arange(1, 26) * (t2**2 - 1j * t1**3)
        cf[2] = np.sin(t1) * cf[0]**2
        cf[6] = np.log(np.abs(t2) + 1) * cf[4]**3
        cf[32] = cf[6] + cf[2]
        cf[36] = cf[32] - cf[6]
        cf[40] = cf[32] + cf[2]
        cf[49] = np.angle(t1) * np.angle(t2)
        cf[50] = np.abs(cf[40])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_9(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(2, 52):
            cf[k-1] = (np.abs(t1) * np.sin(k) + np.angle(t2) * np.cos(k)) / np.abs(k + 1j)
        cf[9] = cf[0]**2 - cf[1]**2 + np.log(np.abs(cf[2]) + 1)
        cf[19] = np.sum(cf[0:19]) * t1
        cf[29] = np.prod(cf[0:29]) * t2
        cf[39] = cf[38] * cf[37] / (1 + t1 * t2)
        cf[40:50] = np.real(cf[30:40]) + 1j * np.imag(cf[0:10])
        cf[50] = np.sum(cf[0:50])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_10(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = np.real(t1 * t2) + np.imag(t2) * np.real(t1)
        cf[1] = np.abs(t1 * t2) * np.cos(np.angle(t1 + t2))
        for i in range(2, 51):
            cf[i] = cf[i - 2] * np.abs(cf[i - 1]) * np.sin(np.angle(t1 + t2))
        cf[50] = np.log(np.abs(t1 * t2)) + cf[0] + cf[1]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_11(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = t1 * t2 - np.log(np.abs(t1 + t2) + 1)
        for i in range(1, 50):
            cf[i] = np.sin(i) * (np.real(t1**i) - np.imag(t2**i)) + np.cos(i) * (np.real(t2**i) - np.imag(t1**i))
            cf[i] = cf[i] / (np.abs(cf[i]) + 1e-10)
        cf[50] = np.abs(t1) * np.abs(t2) * np.angle(t1 + t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_12(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        cf[0] = t1 * (2 * (np.imag(t2))**2)
        cf[1] = t2 * (2 * (np.real(t1))**2)
        for k in range(2, 51):
            cf[k] = ((np.abs(t1)**k + np.abs(t2)**(50-k)) / (k**2)) * np.exp(1j * np.angle(t1 * t2))
        cf[22] = np.cos(t1 * t2) * (t1 - 1j * t2)
        cf[34] = np.sin(t1 * t2) * (1j * t2 - t1)
        cf[49] = np.log(np.abs(t1 + t2))**3
        cf[50] = np.conj(t1) * np.conj(t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_13(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        fib = np.array([1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181])
        for n in range(19):
            cf[n] = fib[n] * t1 * np.cos(np.angle(t2))
            cf[n + 19] = fib[n] * t1 * np.sin(np.angle(t2))
            cf[n + 38] = fib[n] * t2 * np.sin(np.angle(t1))
        cf[19] = np.abs(t1 * t2)
        cf[49] = np.log(np.abs(t1 * t2) + 1)
        cf[50] = np.real(t1) + np.imag(t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_14(t1, t2):
    try:
        cf = np.zeros(51, dtype=np.complex128)
        triangleNums = np.cumsum(np.arange(1, 51))
        cf[0] = t1 + 3 * t2
        for k in range(1, 51):
            cf[k] = triangleNums[k] * (t1 + t2 * np.log(np.abs(t1) + 1))**(k) + triangleNums[k] * (t2 + t1 * np.log(np.abs(t2) + 1))**(k)
        cf[42] = np.real(np.abs(t1)) + np.imag(np.abs(t2))
        cf[20] = np.real(np.abs(t2)) + np.imag(np.abs(t1))
        cf[31] = np.real(np.abs(t1 * t2)) + np.imag(np.conj(t1 * t2))
        cf[27] = 2 * np.real(t1 - t2) + 2 * np.imag(t1 - t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_15(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241])
        for i in range(71):
            cf[i] = (primes[i] * t1 + 1j * t2**i) / (1 + np.abs(t1))**i
        cf[70] = np.sum(cf[0:70])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_16(t1, t2):
    cf = np.zeros(51, dtype=np.complex128)
    cf[0] = t1 + t2
    cf[1] = np.real(t1**2 - t2**2)
    primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
                       53, 59, 61, 67, 71, 73, 79, 83, 89, 97])
    for k in range(2, 25):  # Adjusted indices: R is 1-indexed, Python is 0-indexed
        cf[k] = np.imag(cf[k - 1] * primes[k - 2]) * np.angle(t1) * np.abs(t2)
    for k in range(25, 50):
        cf[k] = np.abs(cf[k - 1] * primes[k - 25] ** 2) * np.angle(t2) * np.real(t1)
    cf[50] = np.sum(cf) + np.sin(np.real(t2)) * np.log(np.abs(t1) + 1)
    return cf.astype(np.complex128)

def poly_17(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:10] = (t1 + t2) * np.arange(1, 11)
        cf[10:20] = np.real(t1 - t2)**3 * np.arange(11, 21)
        cf[20:30] = np.imag(t1 + t2)**2 * np.arange(21, 31)
        cf[30:40] = np.abs(t1 - t2) * np.arange(31, 41)
        cf[40:50] = np.angle(t1 * t2) * np.arange(41, 51)
        cf[50] = np.sin(t1) * np.cos(t2) + np.sin(t2) * np.cos(t1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_18(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            arg = np.angle(t1 * t2 * 1j)
            mod = np.abs((t1 + 1j) * (t2 + 1j))
            cyclotomic = 0
            for k in range(1, i + 1):
                cyclotomic += np.prod(t1 - np.exp(2j * np.pi * k / i))
            cf[i-1] = mod * cyclotomic * arg
        cf[70] = np.log(np.abs(t1 * t2)) + 1
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_19(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.real(t1) + np.imag(t2)
        cf[1] = np.angle(t1)
        cf[2] = np.abs(t2)
        cf[3] = np.sin(t1) + np.cos(t2)
        cf[4:10] = np.arange(1, 11) * 0.2 + 1
        cf[10] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        for i in range(11, 72):
            cf[i-1] = cf[i-2] * np.sin(i * cf[i-3] + np.abs(cf[i-4])) + cf[i-5]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_20(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 18):
            cf[i * 3 - 2] = ((t1 + t2)**i + i * (t1 - t2)**i) / (2**i)
        cf[3] = np.abs(t1) * np.sin(np.angle(t1))
        cf[7] = np.abs(t2) * np.cos(np.angle(t2))
        cf[18] = np.log(np.abs(t1 * t2)) * np.cos(np.angle(t1 - t2))
        cf[36] = np.abs(t1 * t2) * np.cos(np.angle(t1 + t2))
        cf[[20, 24, 28, 32, 36, 40, 44, 48, 50]] = np.real(t1) + np.imag(t2)
        cf[[22, 26, 30, 34, 38, 42, 46]] = np.imag(t1) + np.real(t2)
        cf[49] = np.abs(t1)**2 * np.sin(2 * np.angle(t2))
        cf[50] = np.abs(t2)**2 * np.cos(2 * np.angle(t1))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_21(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        roots = np.exp(2 * np.pi * 1j * np.arange(0, 51) / 51)
        for k in range(1, 72):
            cf[k-1] = np.prod(roots[np.arange(51) != k-1] - roots[k-1]) / (t1 - roots[k-1]) / (t2 - roots[k-1])
        return cf.astype(np.complex128) * (t1 - roots) * (t2 - roots)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_22(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97])
        cf[0:25] = primes[0:25] * (np.real(t1)**2 - np.imag(t1**2) + np.real(t2)**2 - np.imag(t2**2))
        cf[25:50] = cf[0:25] * (np.cos(np.angle(t1 + t2)) + np.sin(np.abs(t1) * np.abs(t2)))
        cf[50] = np.sum(cf[0:50])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_23(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = 1 + t1 * t2 + np.log(np.abs(t1 + t2) + 1)
        cf[2] = t1 + t2 + np.log(np.abs(1 - t1 * t2) + 1)
        for i in range(3, 72):
            cf[i-1] = i * t1 + (51 - i) * t2 + np.log(np.abs(t1 - t2 * i) + 1)
        cf[10] = cf[0] + cf[9] - np.sin(t1)
        cf[20] = cf[30] + cf[40] - np.cos(t2)
        cf[30] = cf[20] + cf[40] + np.sin(t1)
        cf[40] = cf[30] + cf[20] - np.cos(t2)
        cf[50] = cf[40] + cf[20] + np.sin(t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_24(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1 * t2
        for n in range(3, 72):
            cf[n-1] = np.abs(cf[n-2]) + 1j * np.angle(cf[n-3]) + np.abs(t1 + t2)**(1/n) * (np.cos(n * t2) + 1j * np.sin(n * t1))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_25(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.real(t1 * t2) + np.imag(t1 / t2)
        for k in range(1, 72):
            cf[k] = np.abs(t1)**k + np.angle(t2)**k + np.sin(t1 + k) + np.cos(t2 + k) - np.log(np.abs(t1 * t2)**k + 1)
        cf[35] = np.real(cf[0] * cf[34]) + np.imag(t1 * t2)
        cf[45] = 0.5 * (t1 + np.conj(cf[44]) + t2)
        cf[50] = cf[0] + cf[34] + cf[44] + np.real(t1) + np.imag(t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_26(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1 * t2
        for k in range(3, 72):
            v = np.sin(k * cf[k-2]) + np.cos(k * cf[k-3])
            cf[k-1] = v / np.abs(v)
        cf[14] = np.abs(t1 - t2) * np.angle(t1 + t2)
        cf[29] = np.log(np.abs(t1 * np.real(t2) + 1)) - np.log(np.abs(t2 * np.imag(t1) + 1))
        cf[49] = np.prod(np.abs(t1), np.abs(t2)) * np.abs(t1 - t2)
        cf[50] = np.sum(cf[15:29]) * np.sum(cf[30:44]) + t1**2
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_27(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:10] = np.abs(t1) * np.abs(t2) * np.arange(1, 11)
        cf[10:20] = np.abs(t1)**(np.arange(2, 12))
        cf[20:30] = np.abs(t2)**(np.arange(2, 12))
        cf[30] = np.real(t1 * t2)
        cf[31] = np.imag(t1 * t2)
        cf[32:42] = np.real(t1) * np.arange(1, 11) + np.imag(t2) * np.arange(1, 11)
        cf[42:51] = np.real(t2) * np.arange(1, 10) + np.imag(t1) * np.arange(1, 10)
        for i in range(50):
            cf[i] += np.sin(cf[i + 1])
        for i in range(51, 1, -1):
            cf[i - 1] -= np.cos(cf[i - 2])
        cf[50] += np.angle(t1) + np.angle(t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_28(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53])
        for k in range(1, 17):
            cf[k-1] = (np.real(t1) + np.imag(t2)) / primes[k-1]
            cf[71-k] = (np.imag(t1) + np.real(t2)) * primes[k-1]
        for k in range(17, 36):
            cf[k-1] = np.sin((np.real(t1) + np.imag(t1))**2) * (np.real(t2) + np.imag(t2))**(2 + k)
        cf[35] = np.log(np.abs(t1) * np.abs(t2) + 1) + np.abs(t2 - t1)
        cf[36:51] = np.angle(t1 + t2) + np.abs(t1 - t2) + np.angle(np.conj(t1 * t2))
        cf[50] = np.sum(cf[0:50])**2
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_29(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:5] = np.array([1, t1, t1**2, t1**3, t1**4])
        cf[5:10] = np.array([1, t2, t2**2, t2**3, t2**4])
        cf[10:15] = np.array([1, np.exp(1j * t1), np.exp(2j * t1), np.exp(3j * t1), np.exp(4j * t1)])
        cf[15:20] = np.array([1, np.exp(1j * t2), np.exp(2j * t2), np.exp(3j * t2), np.exp(4j * t2)])
        cf[20:30] = np.array([1, np.real(t1 + t2), np.imag(t1 + t2), np.real(t1 * t2), np.imag(t1 * t2), np.real(t1 + t2)**2, np.imag(t1 + t2)**2, np.real(t1 * t2)**2, np.imag(t1 * t2)**2, np.abs(t1 + t2)])
        cf[30:40] = np.arange(1, 11) * np.abs(t1) * np.abs(t2)
        cf[40:50] = np.array([1, np.log(np.abs(t1) + 1), np.log(np.abs(t2) + 1), np.log(np.abs(t1 + t2) + 1), np.log(np.abs(t1 * t2) + 1), np.angle(t1), np.angle(t2), np.abs(t1), np.abs(t2), np.angle(t1 + t2)])
        cf[50] = np.abs(t1 + t2) * np.angle(t1 * t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_30(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i-1] = (np.real(t1) * np.imag(t2) + np.imag(t1) * np.real(t2))**(1/i) * np.abs(t2)**(i/50) * np.sin(np.angle(t1) * (i/25)) * np.cos(np.angle(t2) * (i/50)) + np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_31(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        for i in range(1, 36):
            cf[i] = (np.cos(i * t1) + np.sin(i * t2)) / (np.abs(t1) * np.abs(t2))**i
        for i in range(36, 72):
            cf[i] = (np.cos(t1**i) + np.sin(t2**i)) * np.log(np.abs(t1)**i + 1) * np.log(np.abs(t2)**i + 1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_32(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, 72):
            r = np.abs(t1)**k + np.abs(t2)**(71-k)
            theta = np.angle(t1)**k - np.angle(t2)**(71-k)
            cf[k-1] = r * np.cos(theta) + r * np.sin(theta) * 1j
        cf[2:70] += np.log(np.abs(t2 - t1) + 1)
        cf[70] += np.conj(t1 * t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_33(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        f = lambda z, n: z**n - 1
        cf[0:35] = [np.real(f(t1, n)) - np.imag(f(t2, n)) for n in range(1, 36)]
        cf[35:70] = [np.log(np.abs(f(t2, n))) + np.angle(f(t1, n)) + np.sin(np.abs(f(t1, n))) + np.cos(np.angle(f(t2, n))) for n in range(1, 36)]
        cf[70] = np.prod(cf[0:70])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_34(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i-1] = (t1 * t2) * np.sin(i * t1 + t2) + np.cos(i * t1 - t2) + i * np.log(np.abs(t1 + i * t2) + 1) / (1 + np.abs(t1 + i * t2))
        cf[12] = 3 * (t1**2 - t2**2)
        cf[13] = cf[12] + t1 * t2 * np.sin(np.angle(t1 + t2))
        cf[14] = 2 * cf[13] - t1 * t2 * np.cos(np.angle(t1 - t2))
        cf[15] = 3 * cf[12] - cf[13] + t1 * t2 * np.sin(2 * np.angle(t1 - t2))
        cf[16] = 2 * cf[12] - 3 * cf[13] + cf[14] - t1 * t2 * np.cos(2 * np.angle(t1 + t2))
        cf[69] = 2 * cf[13] - 3 * cf[12] + t1 * t2 * np.sin(np.angle(2 * t1 - t2))
        cf[70] = cf[16] - cf[13] * t1 * t2 * np.cos(np.angle(2 * t1 + t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_35(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 71):
            z = np.cos(t1) * t1**k - np.sin(t2) * t2**k
            cf[k-1] = np.real(z) + 1j * np.imag(z)
        cf[70] = np.abs(t1 * t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_36(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2 * 1j
        cf[2] = np.abs(t1)**2 + np.angle(t2)**2
        cf[8] = np.sin(t1 + t2)
        for k in range(9, 72):
            cf[k-1] = np.cos(k * np.real(t1 + t2)) + np.sin(k * np.imag(t1 * np.conj(t2)))
        cf[70] = np.log(np.abs(t1 * t2) + 1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_37(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i-1] = (np.sin(i * t1) + np.cos(i * t2)) * i**2
        cf[1] += np.sum(cf[0:2])
        cf[4] += np.prod(cf[0:5])
        cf[11] += np.log(np.abs(cf[10]) + 1)
        cf[24] += np.angle(cf[23])
        cf[[34, 44, 54, 64]] += np.abs(t2)**2 + np.real(t1)**3
        cf[[6, 13, 20, 27, 34, 41, 48, 55, 62, 69]] += np.sin(t1)**i - np.cos(t2)**i
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_38(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 71):
            cf[k-1] = (1 + t1 * t2)**k / (1 + np.real(t1 * t2)**2)
        cf[70] = np.abs(t1) + np.abs(t2) + np.angle(t1 + t2) + np.sin(np.real(t1) + np.imag(t2)) + np.log(np.abs(np.real(t2) + 1))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_39(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1
        cf[1] = t2
        for k in range(3, 72):
            cf[k-1] = np.sin(k * t1) + np.cos(k * t2) + np.log(np.abs(k) + 1) * np.abs(cf[k-2]) * np.abs(cf[k-3]) * np.abs(np.angle(t1 + t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_40(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 71):
            cf[k-1] = (np.real(t1)**(k + 1)) * np.sin(np.angle(t2 * k)) + (np.imag(t2)**k) * np.cos(np.angle(t1 / k))
        cf[70] = np.abs(t1) + np.abs(t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_41(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = (t1 + t2) * (t1 - t2)
        cf[2] = np.abs(np.real(t1))**2 + np.abs(np.imag(t1))**2 + np.abs(np.real(t2))**2 + np.abs(np.imag(t2))**2
        for i in range(3, 72):
            cf[i-1] = cf[i-2] * t1 + cf[i-3] * t2 + cf[i-4]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_42(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:35] = np.abs(t1) * np.sin((np.arange(1, 36)) * np.angle(t1))
        cf[35:70] = np.real(t2) * np.cos((np.arange(1, 36)) * np.imag(t2))
        cf[70] = t1 * t2 + 1j * np.sum(np.log(np.abs(cf[0:70]) + 1))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_43(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        for k in range(2, 72):
            cf[k-1] = np.sin(k * cf[k-2]) + np.cos(k * t1) - np.sin(k * cf[k-3]) + np.cos(k * t2)
            cf[k-1] = cf[k-1] / np.abs(cf[k-1])
        cf[34] = np.real(t1)**3 - np.imag(t2)**3
        cf[52] = np.abs(t1 * t2)**2 - np.angle(t1 * t2)
        cf[70] = np.real(t1 * t2) - np.imag(t1 * t2) + np.angle(t1 * t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_44(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.sin(t1) * np.cos(t2)
        cf[1] = np.cos(t1) + np.sin(t2)
        cf[2] = np.abs(t1)**3 - np.abs(t2)**4
        cf[3] = np.angle(t1) - np.angle(t2)
        cf[4] = np.abs(t1 * t2)
        for k in range(6, 36):
            cf[k-1] = np.sin(k * t1) + np.cos(k * t2)
            cf[k + 34] = np.sin((70 - k) * t1) - np.cos((70 - k) * t2)
        cf[35] = np.abs(t1 + t2)
        cf[70] = np.log(np.abs(t1 * t2) + 1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_45(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            if k % 2 == 0:
                cf[k-1] = k * (t1 + np.real(t2)) * np.sin(np.abs(t1) * k)
            else:
                cf[k-1] = k * (t2 - np.imag(t1)) * np.cos(np.angle(t2) * k)
        for i in range(2, len(cf) // 2):
            cf[i-1] = cf[i-2] * (np.abs(t1) + 0.5) + np.log(np.abs(t2) + 1)
            cf[len(cf) - i] = -cf[len(cf) - i + 1] * (np.abs(t2) + 0.5) - np.log(np.abs(t1) + 1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_46(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67])
        cf[0:18] = np.real(primes * (t1 + t2))
        cf[18:36] = np.imag(primes * (t1 - t2))
        cf[36:54] = np.real(primes * (t1 * np.conj(t2)) + np.log(np.abs(primes)))
        cf[54:71] = np.imag(t1**(np.arange(1, 17))) * t2**(np.arange(1, 17)**2)
        cf[70] = np.sum(t1**(np.arange(1, 6)) * t1**(np.arange(1, 6)**2)) + np.sum(t2**(np.arange(1, 11)))**2
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_47(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = np.sin(t1) * np.cos(t2)
        cf[2:6] = np.log(np.abs(t1 + t2) + 1) * np.arange(1, 5)
        cf[6:10] = 2 * np.abs(t1) * np.log(np.abs(t2) + 1) * np.arange(1, 5)
        cf[10:14] = 3 * np.abs(t2) * np.log(np.abs(t1) + 1) * np.arange(1, 5)
        cf[14:18] = np.angle(t1 + t2) * np.arange(1, 5)
        cf[18:26] = np.cos(t1) * np.sin(t2) * np.arange(1, 9)
        cf[26:34] = np.sin(t1) * np.cos(t2) * np.arange(1, 9)
        cf[34:50] = (t1 + t2) / np.arange(1, 17)
        cf[50] = np.prod(np.arange(1, 72))
        cf[51:72] = (t1 - t2) / np.arange(20, 0, -1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_48(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i-1] = np.abs(t1)**(i / (t1 + t2))
        cf[0:10] += np.angle(t2) * 10
        cf[10:20] -= np.angle(t1) * 10
        cf[20:30] += np.real(t1)**2
        cf[30:40] -= np.imag(t2)**2
        cf[40:50] += np.abs(t1) * np.log(np.abs(t1) + 1)
        cf[50:60] -= np.abs(t2) * np.log(np.abs(t2) + 1)
        cf[60:70] += np.sin(t1 + t2)
        cf[70] = np.prod(cf[0:70]) / 70
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_49(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i-1] = (i**2 + 3 * 1j + 1) * (t1**2) + (i**3 - i**2 + 1) * (t2**2) + np.sin(t1 * 1j + t2) + np.log(np.abs(t1 * 1j - t2) + 1)
            if i % 2 == 0:
                cf[i-1] += (t1 + 1j * t2)**2
            elif i % 3 == 0:
                cf[i-1] += np.abs(t1 + 1j * t2)**3
            else:
                cf[i-1] += np.real(t1 + 1j * t2)**4
        cf[0] *= 10000
        cf[1] *= 1000
        cf[2] *= 100
        cf[3] *= 10
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_50(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        complex_val = np.sin(t1 + t2) + np.cos(t1 - t2)
        for k in range(1, 72):
            if k % 2 == 0:
                cf[k-1] = complex_val / np.abs(k) - np.abs(t1)
            else:
                cf[k-1] = complex_val * np.abs(k) + np.log(np.abs(k) + 1) + np.imag(t2) - np.real(t1)
            if k % 3 == 0:
                cf[k-1] += 3 * cf[k-2]
            if k % 5 == 0:
                cf[k-1] += 5 * cf[k-3]
            if k % 7 == 0:
                cf[k-1] += 7 * cf[k-4]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_51(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 71):
            if k % 2 == 0:
                cf[k] = np.sin(k * (t1 + t2) ** k) + np.cos(k * (t1 - t2) ** k)
            else:
                cf[k] = np.real(t1) ** k + np.imag(t2) ** k
        cf[70] = np.abs(t1) ** 3 + np.angle(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_52(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 ** 7 + t2 ** 7
        for k in range(2, 36):
            cf[k - 1] = np.sin(k * np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1))
        for k in range(36, 71):
            cf[k - 1] = np.cos(k * np.log(np.abs(t1) + 1) - np.log(np.abs(t2) + 1))
        cf[70] = t1 * t2 - (t1 + t2) ** 2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_53(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = 1j * (t1 + t2)
        cf[2] = np.real(t1) + np.imag(t2)
        cf[3] = np.sin(cf[0]) * np.cos(cf[1])
        cf[4] = np.abs(t1 - t2)
        for k in range(6, 37):
            cf[k - 1] = np.abs(cf[k - 2]) ** 2 - np.log(np.abs(cf[k - 3]) + 1)
        for k in range(36, 72):
            cf[k - 1] = np.angle(cf[k - 2] * cf[k - 4]) * np.abs(cf[k - 3] * cf[k - 6])
        cf[70] = np.real(t1) * np.imag(t2) - np.real(t2) * np.imag(t1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_54(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            z = t1 * np.cos(i * t2 / 15) + t2 * np.sin(i * t1 / 15)
            phi = np.angle(z)
            r = np.abs(z)
            cf[i - 1] = r * np.exp(1j * phi) ** i + (-1) ** (i + 1) * i ** 2
        cf[0:30] = cf[0:30] * (np.abs(t1) * np.abs(t2)) ** np.arange(1, 31)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_55(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = np.real(t1) * np.imag(t2)
        cf[2] = np.real(t2) * np.imag(t1)
        cf[3:10] = np.linspace(np.log(np.abs(cf[0]) + 1), np.log(np.abs(cf[2]) + 1), 7)
        cf[10:30] = [np.cos(cf[i - 1]) + ((t1 + t2) ** i) / (i + 1) for i in range(11, 31)]
        cf[30:50] = [np.sin(cf[i - 1]) + ((t1 - t2) ** i) / (i + 1) for i in range(31, 51)]
        cf[50:70] = np.abs(cf[0:20]) + np.abs(cf[20:40] + t1 + t2)
        cf[70] = np.prod(cf[0:70])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_56(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = ((t1 + 1j * t2) ** k + (t2 + 1j * t1) ** (71 - k)) / 2
        cf[3:68] = cf[3:68] * (1 + np.sin(np.angle(t1 + 1j * t2)))
        cf[0:3] = cf[0:3] * (1 + np.cos(np.angle(t1 + 1j * t2)))
        cf[68:71] = cf[68:71] * np.abs(t1 + 1j * t2)
        cf[34] = cf[34] * np.log(np.abs(np.imag(t1 + 1j * t2)) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_57(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 36):
            cf[k - 1] = (t1 * k ** 2 + t2 * (70 - k)) * (1 - (-1) ** k) / 2
        for k in range(36, 71):
            cf[k - 1] = (t1 * np.conj(t2)) ** k * np.abs(t1 - t2)
        cf[70] = np.abs(t1 * np.real(t2)) * np.abs(t2 - t1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_58(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = 100 * np.abs(t1) * np.abs(t2) - 100
        cf[1] = 100 * t1 ** 3 - t2 ** 3 + 100
        cf[2] = 100 * t2 ** 3 - t1 ** 3 + 100
        cf[3:71] = [np.cos(k * t1) * np.sin(k * t2) / np.log(np.abs(k + 1)) for k in range(1, 69)]
        root_coeff = np.abs(t1) * np.abs(t2) * np.prod(range(1, 71)) / np.sum(range(1, 71))
        cf[4] = root_coeff * np.sum([np.cos(k * t1) * np.sin(k * t2) / np.log(np.abs(k + 1)) for k in range(1, 71)])
        cf[35] = root_coeff * t1 ** 2
        cf[34] = root_coeff * t2 ** 2
        cf[36:71] = root_coeff * [np.cos(2 * k * t1) * np.sin(2 * k * t2) / np.log(np.abs(2 * k + 1)) for k in range(1, 36)]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_59(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i - 1] = (t1 / (i + 1)) ** i + (t2 / (i + 1)) * (2j)
        cf[[1, 3, 5, 7, 9, 11, 13, 15, 17, 19]] *= (t1 + 2 * t2)
        cf[[2, 5, 8, 11, 14, 17, 20, 23, 26, 29]] *= (t1 - 2 * t2)
        cf[4:36] += 2 * t1
        cf[36:67] -= 2 * t2
        cf[67:71] = np.real(np.log(cf[67:71])) + np.sum(cf[4])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_60(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = t1 ** k + np.conj(t2) ** (k - 1) / ((k % 2) + 1)
        cf[0:5] = np.abs(cf[0:5]) ** 2
        cf[5:10] *= np.log(np.abs(t2) + 1)
        cf[10:15] *= np.abs(t1 - t2) ** 3
        cf[15:20] *= np.sin(t1 + t2)
        cf[20:25] *= np.cos(t1 - t2)
        cf[25:30] *= np.abs(t1 + t2) ** 2
        cf[30:35] *= np.sin(t1 - t2)
        cf[35:40] *= np.cos(t1 + t2)
        cf[40:45] *= np.abs(t1 - t2)
        cf[45:50] *= np.sin(t1 + t2)
        cf[50:55] *= np.cos(t1 - t2)
        cf[55:60] *= np.abs(t1 + t2)
        cf[60:65] *= np.sin(t1 - t2)
        cf[65:70] *= np.cos(t1 + t2)
        cf[70] *= np.abs(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_61(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:35] = np.real(t1) * (np.arange(1, 36) ** 3) + np.imag(t2) * np.sin(np.arange(1, 36))
        cf[35:70] = np.imag(t1) * (np.arange(70, 35, -1) ** 2) + np.real(t2) * np.cos(np.arange(70, 35, -1))
        cf[70] = np.abs(t1) * np.angle(t2) - np.abs(t2) * np.angle(t1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_62(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 ** 5 + t2 ** 5
        for i in range(2, 72):
            if i % 2 == 0:
                cf[i - 1] = i * cf[i - 2] ** 2
            else:
                cf[i - 1] = i * cf[i - 2] ** 2 * (1 + 0.1 * t2)
        cf[0] += 2 * cf[1]
        cf[1] -= 3 * cf[2]
        for i in range(3, 70):
            cf[i] += cf[i + 1] - cf[i + 2]
        cf[69] += cf[70]
        cf[70] = np.abs(t1) ** 2 - np.abs(t2) ** 2 + 2 * np.imag(t1) * np.imag(t2) - np.angle(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_63(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(1, 72):
            cf[j - 1] = np.sin(t1 ** j) * np.cos(t2 ** (71 - j)) * np.abs(t1 * t2 ** j) * np.log(np.abs(t1 * t2 + 1))
        cf[0:30] += cf[30:60]
        cf[32:72] -= cf[0:40]
        cf[10:60] += np.real(t1) * np.imag(t2) * cf[0:50]
        cf[30:70] -= np.imag(t1) * np.real(t2) * cf[1:41]
        cf[20:40] += np.angle(t1 ** t2) * cf[30:50]
        cf[40:72] -= np.angle(t2 ** t1) * cf[0:32]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_64(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = (np.abs(t1) ** k + np.abs(t2) ** (71 - k)) * np.cos(np.angle(t1) * k + np.angle(t2) * (71 - k))
        cf[1::2] *= 1j
        cf[2::3] *= -1
        cf[0] *= 100
        cf[70] /= 100
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_65(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = (t1 + t2) ** (2 * k - 1) + np.sin(k * t1) * np.cos(k * t2) + np.log(np.abs(k ** t2) + 1) * np.real(t1 ** t2) + np.abs(np.imag(t1 ** (2 * k + 1) + t2 ** (2 * k)))
            cf[k - 1] = np.conj(cf[k - 1]) * (-1) ** k
            if k % 2 == 0:
                cf[k - 1] /= (k + t1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_66(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:25] = np.real(t1) * np.log(np.abs(t2) + 1) * (np.arange(1, 26) ** 2)
        cf[25:50] = np.imag(t2) * np.log(np.abs(t1) + 1) * (np.arange(1, 26) ** 3)
        cf[50:70] = np.abs(t1) * np.abs(t2) * np.log(np.abs(t1 + t2) + 1) * (np.arange(1, 21))
        cf[70] = np.sum(cf[0:70]) * np.angle(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_67(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 ** 7 + 1j * t2 ** 8
        cf[1] = 2 * t1 ** 6 - 1j * t2 ** 7
        cf[2] = 3 * t1 ** 5 + 3j * t2 ** 6
        cf[3] = 5 * t1 ** 4 - 5j * t2 ** 5
        cf[4] = 7 * t1 ** 3 + 7j * t2 ** 4
        cf[5] = 11 * t1 ** 2 - 11j * t2 ** 3
        cf[6] = 13 * t1 + 13j * t2
        cf[70] = np.abs(t1 * t2) ** 2 * np.angle(t1 * t2) + np.sin(np.real(t1)) - np.cos(np.imag(t2))
        for k in range(8, 71):
            cf[k] = np.real((t1 + 1j * t2) ** (70 - k)) - np.imag((t1 - 1j * t2) ** (k - 1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_68(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:5] = np.abs(t1) ** (np.arange(1, 6))
        for i in range(6, 71):
            cf[i] = (i * t1 + 2 * i * t2) / (i + 1)
        cf[70] = np.abs(t1) * np.abs(t2) * np.angle(t1) * np.angle(t2) * np.sin(np.abs(t1 + t2))
        cf[20:30] += np.log(np.abs(t1 + t2) + 1) * np.exp(1j * np.pi / 10 * np.arange(1, 11))
        cf[50:60] += 1j * (cf[0:10] / np.arange(11, 21))
        cf[60:70] -= np.sin(cf[0:10])
        cf[30:40] += np.cos(t1 + t2) ** np.arange(1, 11)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_69(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            j = 71 - i
            cf[i - 1] = ((np.real(t1) + np.imag(t1) * j) / np.abs(t2 + i)) * np.sin(np.angle(t1 + t2 * i)) + np.log(np.abs(t1 * t2) + 1) * np.cos(2 * np.pi * i / 71)
        cf[cf == 0] = np.real(t1) ** 2 - np.imag(t1) * np.imag(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

#  cf = complex(71)
#   for (i in 1:71) {
#     cf[i] = Re(t1) * Re(t2) * (i^(2))/(exp(np.abs(t1)*1i)) + Im(t1) * Im(t2) * (i^3)/(exp(np.abs(t2)*1i))
#   }
#   cf[2:2:length(cf)] = cf[2:2:length(cf)] * (-1)
#   p = 1:71
#   cf[p^2 <= 71] = cf[p^2 <= 71]+1i*Mod(t1)*Mod(t2)
#   cf
def poly_70(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i - 1] = np.real(t1) * np.real(t2) * (i ** 2) / np.exp(np.abs(t1) * 1j) + np.imag(t1) * np.imag(t2) * (i ** 3) / np.exp(np.abs(t2) * 1j)
        cf[1::2] *= -1
        p = np.arange(1, 72)
        cf[p ** 2 <= 71] += 1j * np.abs(t1) * np.abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_71(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i - 1] = ((np.real(t1) ** i * np.imag(t2) ** i) + (np.imag(t1) ** i * np.real(t2) ** i)) / (i ** 2 + 1)
        cf[0:5] *= 1000
        cf[8:15] *= -500
        cf[17:25] *= 250
        cf[29:35] *= -125
        cf[36:45] *= 60
        cf[49:54] *= -30
        cf[55:64] *= 15
        cf[69:71] *= -5
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_72(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1:71] = np.arange(1, 71) * (t1 - t2 + (np.sin(np.arange(1, 71)) + 1j * np.cos(np.arange(1, 71))))
        roots = np.abs(cf[1:71])
        sorted_roots = np.sort(roots)[::-1]
        cf[1:71] = sorted_roots * (t1 + t2 * 1j * np.arange(1, 71))
        cf = np.real(cf) + np.imag(cf) + (np.sin(np.angle(cf + 1)) + 1j * np.cos(np.angle(cf + 1)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_73(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = 10 ** 30 * (t1 + t2)
        cf[1] = 10 ** 28 * (t1 - t2)
        cf[2] = 10 ** 26 * (t1 + t2)
        for k in range(4, 22):
            cf[k - 1] = 10 ** (30 - k) * (np.cos(t1) + np.sin(t2))
        for k in range(22, 32):
            cf[k - 1] = 10 ** (k - 21) * (np.cos(t1) - np.sin(t2))
        for k in range(32, 42):
            cf[k - 1] = 10 ** (42 - k) * (t1 + t2) * (np.cos(t1 + t2) + np.sin(t1 - t2))
        cf[41] = 10 ** 21 * (t1 - t2)
        for k in range(43, 54):
            cf[k - 1] = 10 ** (53 - k) * (np.abs(t1 + t2) + np.angle(t1 - t2))
        for k in range(54, 65):
            cf[k - 1] = 10 ** (64 - k) * (np.abs(t1 - t2) + np.angle(t1 + t2))
        for k in range(65, 72):
            cf[k - 1] = 10 ** (71 - k) * (np.sin(t1) + np.cos(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_74(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 36):
            cf[i - 1] = i * (t1 + i * t2) ** (1 / i)
            cf[70] = np.conj(cf[i - 1])
        cf[35] = 2 * t1 + 3 * np.abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_75(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        powers = np.arange(0, 71)
        cf[0] = 100 * t1 ** 3 + 110 * t1 ** 2 + 120 * t2 - 130
        cf[1] = 200 * t2 ** 3 - 210 * t2 ** 2 + 220 * t2 - 230
        cf[4] = np.abs(t1) ** 4
        cf[9] = np.angle(t2) ** 6
        cf[14] = np.log(np.abs(t1 + 1j * t2)) + 1
        cf[[19, 39]] = np.real(1j * cf[4] * t1 * t2)
        cf[[29, 59]] = np.imag(cf[1] * np.conj(cf[0]))
        cf[34] = np.sin(cf[1]) + np.cos(cf[0])
        cf[2] = np.abs(cf[9]) ** 2
        cf[3] = np.prod(cf[[2, cf[1]]])
        cf[8] = np.sum(cf[[19, 39, 59]])
        cf[15:71] = powers[15:71] * np.abs(t1 - t2)
        cf[70] = np.prod(cf[0:4])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_76(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 ** 7
        cf[1] = t2 ** 7
        cf[2] = t1 ** 6
        cf[3] = t2 ** 6
        cf[4] = t1 ** 5
        cf[5] = t2 ** 5
        cf[6] = t1 ** 4
        cf[7] = t2 ** 4
        for k in range(9, 23):
            cf[k - 1] = np.sin((t1 + t2) / (k - 8)) ** k
        for k in range(23, 37):
            cf[k - 1] = np.cos((t1 - t2) / (k - 22)) ** k
        for k in range(37, 51):
            cf[k - 1] = np.cos((t1 + 1j * t2) / (k - 36)) ** k
        for k in range(51, 65):
            cf[k - 1] = np.sin((t1 - 1j * t2) / (k - 50)) ** k
        for k in range(65, 72):
            cf[k - 1] = (t1 + 1j * t2) ** (k - 64)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_77(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            if k % 2 == 0:
                cf[k - 1] = (np.log(np.abs(t1) + 1) ** k + np.log(np.abs(t2) + 1) ** (71 - k)) * np.sin(k * t1 + (71 - k) * t2)
            else:
                cf[k - 1] = (np.log(np.abs(t1) + 1) ** k - np.log(np.abs(t2) + 1) ** (71 - k)) * np.cos(k * t1 - (71 - k) * t2)
        r = np.abs(t1) * np.abs(t2)
        for k in range(50, 72):
            cf[k - 1] *= (r ** (k - 50))
        for k in range(15, 36):
            cf[k - 1] *= 2 * (r ** (71 - k))
        cf = np.real(cf) + 1j * np.imag(np.conj(cf))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_78(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:10] = np.abs(t1) ** (np.arange(1, 11) / 5) * np.log(1 + np.abs(t2))
        cf[10:20] = np.real(t1) ** (np.arange(1, 11)) * np.angle(t2) * (-1) ** (np.arange(1, 11))
        cf[20:30] = np.imag(t1) ** (np.arange(1, 11) / 3) * np.abs(t2) ** (np.arange(1, 11) / 4) * (-1) ** (np.arange(1, 11))
        cf[30:40] = np.abs(t1 * t2) ** (np.arange(1, 11) / 2) * (np.arange(1, 11))
        cf[40:50] = np.real((t1 + t2) ** (np.arange(1, 11) / 2)) * np.cos(np.angle(t1 * t2)) * (-1) ** (np.arange(1, 11))
        cf[50:60] = np.imag((t1 + t2) ** (np.arange(1, 11) / 3)) * np.sin(np.angle(t1 - t2)) * (-1) ** (np.arange(1, 11))
        cf[60:70] = np.real(t1 ** (np.arange(1, 11))) * np.abs(t2 ** (np.arange(1, 11))) * np.log(1 + np.abs(t1 + t2))
        cf[70] = np.abs(t1 - t2) * np.log(1 + np.abs(t1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_79(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:35] = (np.arange(1, 36)) * (t1 + t2) * np.abs(t1) ** (np.arange(1, 36))
        cf[35:70] = (np.arange(35, 0, -1)) * (t1 - t2) * np.abs(t2) ** (np.arange(35, 0, -1))
        cf[70] = np.abs(t1) * np.abs(t2) + np.imag(t1 * np.conj(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_80(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 ** 70 + t2 ** 70
        cf[1:70] = np.sin(t1 * t2 * (np.arange(1, 70))) ** 2
        cf[13:28] = np.log(np.abs(t2) + 1) ** 2 * cf[13:28]
        cf[30:46] *= np.log(np.abs(t1) + 1)
        for i in range(2, 5):
            cf[i * 15] += i * np.abs(t1) * np.abs(t2)
        cf[70] = np.real(t1) ** 3 - np.imag(t2) ** 2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_81(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = (k + 20) * np.sin(t1 * k) * np.cos(t2 * k) + np.abs(t1) ** k + np.abs(t2) ** k
        cf[np.arange(2, 71, 5)] += np.abs(t1) * np.abs(t2)
        cf[np.arange(3, 70, 7)] += ((-1) ** (np.arange(1, 11))) * np.angle(t1 + t2)
        cf[np.arange(6, 67, 9)] += ((-1) ** (np.arange(1, 8))) * np.log(np.abs(t1 + t2) + 1)
        cf[np.arange(5, 71, 7)] *= np.real(t1 + t2)
        cf[np.arange(7, 64, 11)] *= np.imag(t1 + t2)
        cf[np.arange(1, 72, 7)] *= np.conj(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_82(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = 1 + t1 * t2
        cf[1] = 2 + np.abs(t1) * np.abs(t2)
        cf[2] = 3 + np.abs(t1 + t2)
        for i in range(4, 37):
            cf[i - 1] = i + cf[i - 2] * np.sin(i * t2) + cf[i - 3] * np.cos(i * t1) + cf[i - 4] * np.log(np.abs(i * t1 * t2 + 1))
        for i in range(37, 71):
            cf[i - 1] = 70 - i + cf[70 - min(i, 69)] * np.sin((70 - i) * t1) + cf[69 - min(i, 68)] * np.cos((70 - i) * t2) + cf[68 - min(i, 67)] * np.log(np.abs((70 - i) * t1 * t2 + 1))
        cf[70] = np.sum(cf[0:70]) + np.real(np.angle(t1 - t2)) + np.imag(np.angle(t1 + t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_83(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for i in range(1, 72):
            cf[i - 1] = ((t1 + t2) ** (i - 1)) * np.sin(i) + ((t1 - t2) ** (70 - i + 1)) * np.cos(i)
        cf[0] = np.real(cf[0]) + 1j * np.imag(cf[70])
        cf[70] = np.real(cf[70]) + 1j * np.imag(cf[0])
        cf[35] += np.log(np.abs(t1 * t2)) ** 2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_84(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.real(t1) + 1000j
        cf[1] = np.log(1 + np.abs(t1 + t2)) * 1000
        for k in range(3, 36):
            cf[k - 1] = (-1) ** k * (np.real(t1 ** k) + np.imag(t2 ** k)) * 1000 / (k ** 2)
        for k in range(36, 71):
            cf[k - 1] = (-1) ** (k + 1) * (np.abs(t1) ** (70 - k) + np.abs(np.sin(t1 + t2))) / (k ** 2)
        cf[70] = np.abs(t1) + np.cos(np.angle(t2)) * 1000
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_85(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = np.real(t1 ** k + t2 ** (k - 1)) + np.imag((t1 + t2) ** (70 - k))
            if k % 2 == 0:
                cf[k - 1] += np.sin(np.angle(t1 + t2) * k)
            else:
                cf[k - 1] += np.cos(np.angle(t1 + t2) ** k)
            if k % 3 == 0:
                cf[k - 1] *= np.abs(t1 - t2) ** (k / 10)
            if k % 4 == 0:
                cf[k - 1] += np.log(np.abs(t1) + 1) ** k
            if k % 5 == 0:
                cf[k - 1] -= np.log(np.abs(t2) + 1) ** (71 - k)
        cf[35] *= t1 * t2
        cf[65] *= np.conj(t1) * np.conj(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_86(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = np.cos(k * t1) ** k + 1j * np.sin(k * t2) ** k
        cf[1::2] **= -1
        cf[2::3] **= -2
        for r in range(5, 66, 5):
            cf[r - 1] = (t1 * t2) ** r
        cf[70] = (np.abs(t1) ** 2 + 2 * np.real(t1) * np.imag(t2) + 3 * np.abs(t2) ** 2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_87(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 36):
            cf[k - 1] = (t1 + 1j * t2) ** k + np.log(np.abs(t1 + k * t2) + 1) * np.real(t1 * t2)
            cf[70 - k] = k * (t1 - 1j * t2) ** k - np.log(np.abs(t2 - k * t1) + 1) * np.imag(t1 * t2)
        cf[35] = 100 * np.abs(t1) * np.abs(t2)
        cf[36] = 200 * np.angle(t1) * np.angle(t2)
        cf[37:72] = cf[0:34] - cf[37:72]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_88(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1)
        for k in range(2, 37):
            cf[k - 1] = np.sin(k * t1) + np.cos(k * t2) - k ** 2
        for k in range(37, 71):
            cf[k - 1] = np.sin((71 - k) * t1) - np.cos((71 - k) * t2) + (71 - k) ** 2
        cf[70] = np.real(t1 * t2) + np.imag(t1 * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_89(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 * t2 - 1j * np.abs(t2 - t1)
        for k in range(2, 7):
            cf[k - 1] = cf[k - 2] + np.sin(k * t1) + np.cos(k * t2)
        for k in range(7, 12):
            cf[k - 1] = cf[k - 2] + np.log(np.abs(t1 - k)) - np.log(np.abs(t2 - k))
        for k in range(12, 17):
            cf[k - 1] = cf[k - 2] - np.sin(k * t1) - np.cos(k * t2)
        for k in range(17, 22):
            cf[k - 1] = cf[k - 2] - np.log(np.abs(t1 - k)) + np.log(np.abs(t2 - k))
        for k in range(22, 27):
            cf[k - 1] = cf[k - 2] + np.sin(k * t1) + np.cos(k * t2)
        for k in range(27, 32):
            cf[k - 1] = cf[k - 2] + np.log(np.abs(t1 - k)) - np.log(np.abs(t2 - k))
        for k in range(32, 37):
            cf[k - 1] = cf[k - 2] - np.sin(k * t1) - np.cos(k * t2)
        for k in range(37, 42):
            cf[k - 1] = cf[k - 2] - np.log(np.abs(t1 - k)) + np.log(np.abs(t2 - k))
        for k in range(42, 47):
            cf[k - 1] = cf[k - 2] + np.sin(k * t1) + np.cos(k * t2)
        for k in range(47, 52):
            cf[k - 1] = cf[k - 2] + np.log(np.abs(t1 - k)) - np.log(np.abs(t2 - k))
        for k in range(52, 57):
            cf[k - 1] = cf[k - 2] - np.sin(k * t1) - np.cos(k * t2)
        for k in range(57, 62):
            cf[k - 1] = cf[k - 2] - np.log(np.abs(t1 - k)) + np.log(np.abs(t2 - k))
        for k in range(62, 67):
            cf[k - 1] = cf[k - 2] + np.sin(k * t1) + np.cos(k * t2)
        for k in range(67, 72):
            cf[k - 1] = cf[k - 2] + np.log(np.abs(t1 - k)) - np.log(np.abs(t2 - k))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_90(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 ** 3 - t2 ** 2
        cf[1] = np.real(3 * t1 ** 2 * t2 - t2 ** 3)
        cf[2] = np.imag(3 * t1 * t2 ** 2 - t1 ** 3)
        cf[3] = 4 * t1 ** 2 - 6 * t1 * t2 + 4 * t2 ** 2
        for k in range(5, 72):
            cf[k - 1] = np.abs(t1 * t2) * np.sin(k * t1 + t2) + np.cos(k * np.conj(t1 + t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_91(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 71):
            cf[k - 1] = t1 ** k * t2 ** k
        cf[0] = np.log(np.abs(t1 + t2)) + 1
        cf[1] = np.log(np.abs(t1 * t2)) + 1
        cf[3] = np.abs(t1) ** 2 + np.abs(t2) ** 2
        cf[5] = np.abs(t1) ** 3 - np.abs(t2) ** 3
        cf[7] = np.abs(t1) ** 4 + np.abs(t2) ** 4
        cf[9] = t1 ** 5 - t2 ** 5
        cf[19] = np.abs(t1) * np.sin(np.angle(t2))
        cf[29] = np.abs(t2) * np.real(t1)
        cf[39] = np.abs(t1) * np.imag(t2)
        cf[49] = np.abs(t2) * np.angle(t1)
        cf[59] = np.abs(t1 * t2) * np.cos(np.angle(t2))
        cf[69] = np.abs(t2 * t1) * np.real(t1)
        cf[70] = np.sum(cf[0:70])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_92(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = t1 ** k + t2 ** (71 - k)
        cf[14:25] = 1j * cf[14:25]
        cf[29:45] = np.conj(cf[29:45])
        cf[45:70] = -cf[45:70]
        cf[54:71] = cf[54:71] * (1 + 2j)
        cf[70] = np.real(t1) ** 3 + np.imag(t2) ** 3 - np.log(np.abs(t1) * np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_93(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 36):
            cf[k - 1] = k * (t1 ** (2 * k)) * (t2 ** (2 * (35 - k))) / np.sin(k * np.pi / 180)
            cf[71 - k] = k * (t2 ** (2 * k)) * (t1 ** (2 * (35 - k))) / np.cos(k * np.pi / 180)
        cf[35] = 100 * np.real(t1) * np.imag(t2) + 100 * np.imag(t1) * np.real(t2)
        cf[70] = np.abs(t1 + t2) * np.angle(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_94(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[3] = -t1 ** 9
        cf[5] = t2 ** 8
        cf[7] = -t1 ** 7
        cf[9] = t2 ** 6
        cf[10] = -t1 ** 5
        cf[12] = t2 ** 4
        cf[14] = -t1 ** 3
        cf[16] = t2 ** 2
        cf[20] = -t1
        cf[30] = 5e5
        cf[40] = -5e6
        cf[50] = 5e7
        cf[60] = -5e8
        cf[70] = 5e9
        multiplier = np.sin(t1) * np.sin(t2) - np.cos(t1) * np.cos(t2)
        cf[3:71] *= multiplier
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)



def poly_95(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.real(t1) ** 3 - np.imag(t1) ** 2 + 2 * np.imag(t1) * np.real(t1) - np.real(t2) + np.imag(t2) ** 2
        cf[1] = np.imag(t1) ** 3 - 5 * np.real(t1) ** 2 + 2 * np.real(t1) * np.imag(t1) + 5 * np.real(t2) - 2 * np.imag(t2) ** 2
        for k in range(3, 72):
            cf[k - 1] = np.abs(np.sin(k * t1)) + np.abs(np.cos(k * t2)) - np.abs(t1 ** k + t2 ** (k - 1))
        cf[29:40] = np.abs(cf[29:40]) / (np.abs(t1 - t2) ** 2 + 1)
        cf[49:60] = -np.abs(cf[49:60]) / (np.abs(t1 + t2) ** 2 + 1)
        cf[64:71] = cf[0:7] * (np.abs(t1) ** 2 + np.abs(t2) ** 2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_96(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 ** 5 - t2 ** 4 + t1 ** 2 - t2 ** 2 + np.abs(t1) + np.abs(t2)
        cf[50] = t2 ** 6 - t1 ** 4 + t2 ** 3 - t1 ** 2 + np.angle(t1) + np.sin(t2)
        cf[70] = t1 ** 7 + t2 ** 5 - t1 ** 3 - t2 ** 2 + np.cos(t1) - np.sin(t2)
        for k in range(2, 51):
            cf[k - 1] = k * cf[k - 2] + np.abs(cf[0]) / k
        for r in range(52, 71):
            cf[r - 1] = r * cf[r - 2] + np.abs(cf[50]) / r
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_97(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = (t1 + 1j * t2) * k ** (-np.abs(t1) * np.log(np.abs(k + 1)))
        for k in range(1, 11):
            cf[k - 1] = (t1 + 1j * t2) * np.abs(k ** 3 * np.cos(np.imag(t1 + k * t2)) - np.sin(np.real(t1 - k * t2)))
        for k in range(61, 72):
            cf[k - 1] = (t1 + 1j * t2) * np.abs(k * cf[k - 2]) / np.abs(k ** 3 * np.cos(np.imag(t1 + k * t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_98(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 36):
            cf[k - 1] = (t1 + t2) * np.sin(t2 * k) / k ** 2
        for k in range(36, 71):
            cf[k - 1] = (t1 - t2) * np.cos(t1 * (71 - k)) / (71 - k) ** 2
        cf[70] = np.real(t1) * np.imag(t2) - np.real(t2) * np.imag(t1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_99(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2 + 1
        cf[1:10] = np.real(t1) ** 2 + np.imag(t2) ** 2
        cf[10:20] = np.real(t2) ** 2 + np.imag(t1) ** 2
        cf[20:30] = np.abs(t1 * t2) ** 2
        cf[30:40] = np.abs(t1 + t2) ** 2
        cf[40:50] = np.abs(t1) * np.abs(t2)
        cf[50:60] = np.angle(t1) + np.angle(t2)
        cf[60:70] = np.sin(t1 + t2)
        cf[70] = np.cos(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_100(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        iter = complex(1)
        for j in range(1, 72):
            cf[j - 1] = iter
            iter *= (np.log(np.abs(t1 + 1j * t2) + 1) / (71 - j + 1) + np.conj(iter))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_101(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:10] = 1000 * np.arange(1, 11)
        cf[10:71] = 1
        cf[14] = -1 * np.abs(t1)**3
        cf[29] = -1 * np.abs(t2)**4
        cf[44] = np.abs(t1)**5
        cf[59] = np.abs(t2)**6 
        cf[19] = np.abs(t1)**2 * np.sin(np.angle(t1))
        cf[39] = np.abs(t2)**3 * np.cos(np.angle(t2))
        cf[24] = np.log(np.abs(t1) + 1) * np.abs(t1)
        cf[49] = np.log(np.abs(t2) + 1) * np.abs(t2)
        for j in range(1, 36):
            cf[2*j] = cf[2*j] * (np.sin(j * t1) + np.cos(j * t2)) + cf[2*j + 1] * (np.cos(j * t1) + np.sin(j * t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_102(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = 1000 * (t1 + t2)**2
        for k in range(1, 20):
            cf[k] = (k + 1) * cf[k - 1] + np.sin((k + 1) * t1) + np.cos((k + 1) * t2)
        for k in range(20, 40):
            cf[k] = (k + 1) * cf[k - 1] - np.sin((k + 1) * t1) - np.cos((k + 1) * t2)
        for k in range(40, 60):
            cf[k] = (k + 1) * cf[k - 1] + np.sin((k + 1) * t1 * t2) + np.cos((k + 1) * t1 * t2)
        for k in range(60, 70):
            cf[k] = (k + 1) * cf[k - 1] - np.sin((k + 1) * t1 * t2) - np.cos((k + 1) * t1 * t2)
        cf[70] = np.abs(cf[69]) + np.angle(t1) - np.angle(t2) + np.real(t1 * t2) - np.imag(np.conj(t1) * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_103(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t2 * np.log(np.abs(t1) + 1)
        cf[1] = t1 * np.log(np.abs(t2) + 1)
        
        for k in range(2, 32, 2):
            cf[k] = np.sin(k * t1) * np.log(np.abs(t2) + 1)
            cf[k + 1] = np.cos(k * t2) * np.log(np.abs(t1) + 1)
        
        for k in range(32, 52, 2):
            cf[k] = np.cos(k * t1) * np.log(np.abs(t2) + 1)
            cf[k + 1] = np.sin(k * t2) * np.log(np.abs(t1) + 1)
        
        for k in range(52, 72, 2):
            cf[k] = t1 * np.log(np.abs(t2 * (k + 1)) + 1)
            cf[k + 1] = t2 * np.log(np.abs(t1 * (k + 1)) + 1)
        
        mod_cf = (71 - np.arange(1, 72)) * np.abs(cf)
        arg_cf = np.arange(1, 72) / 71 * np.angle(cf)
        cf = mod_cf * np.exp(1j * arg_cf)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_104(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = ((np.abs(t1)**(1/k)) * (np.cos(np.angle(t1)) + 1j * np.sin(np.angle(t1))) + 
                          (np.abs(t2)**(1/k)) * (np.cos(np.angle(t2)) + 1j * np.sin(np.angle(t2)))) / k
        cf[np.arange(0, 71, 3)] *= -1
        cf[np.arange(1, 71, 4)] *= 2
        cf[np.arange(2, 71, 5)] *= 3
        cf[np.arange(3, 71, 6)] *= 4
        cf[np.arange(4, 71, 7)] *= 5
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_105(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = np.sin(k * (np.real(t1) * np.imag(t2))**3) + np.cos(k * np.log(np.abs(t1 * t2 + 1)) * np.angle(t1 + np.conj(t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_106(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = ((t1**3 + t2**2)**2 + np.abs(t1 - t2) + np.sin(t1 * t2)) * np.abs(t1 + t2)**(1/k)
        cf[0] *= 100
        cf[1] *= 90
        cf[2] *= 80
        cf[3] *= 70
        cf[4] *= 60
        cf[5] *= 50
        cf[6] *= 40
        cf[7] *= 30
        cf[8] *= 20
        cf[9] *= 10
        cf[11] *= 5
        cf[23] *= 4
        cf[35] *= 3
        cf[47] *= 2
        cf[59] *= 1
        
        for k in range(15, 72):
            cf[k] = -cf[k] * np.log(np.abs(k))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_107(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)

        for r in range(1, 72):
            cf[r - 1] = (100 * (t1 ** (71 - r))) * np.sin(0.5 * t1 * r) + \
                         (100 * (t2 ** r)) * np.cos(0.5 * t2 * r)
        
        cf[14] = 100 * t2**3 - 100 * t2**2 + (100 * t2 - 100)
        cf[29] = 100 * np.log(np.abs(t1 * t2) + 1)
        cf[44] = np.abs(10 * t1 + 0.5 * t2)
        cf[59] = np.angle(0.2 * t1 - 3j * t2)
        cf[70] = np.real(10 * t1 + 0.5 * t2)

        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_108(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = -(t1**2 + t2)
        cf[2] = t1**2 - t2**2 - 1j
        cf[3:10] = [1 - t1, -1 + t2, 2 - t1, -2 + t2, 3 - t1, -3 + t2, 4 - t1]
        cf[10] = 15 * (np.real(t1) + np.imag(t2))
        cf[11] = -17 * np.angle(t1) * np.angle(t2)
        cf[14] = 30 * np.abs(t1) * np.abs(t2)
        cf[16] = -(t1**3 + t2**3)
        cf[18] = (t1**2 - t2**2) * 1j
        cf[19] = 5 + 1j * t1
        cf[24] = 50 * np.abs(t1 - t2)
        cf[29] = -40 * np.real(t1) + 35 * np.imag(t2)
        cf[34] = np.sum([3, 3, 9, 15, -12]) * (np.real(t1) - np.imag(t2))
        cf[39] = -t1**4 + t2**4 - 3
        cf[44] = 3 * np.angle(t1) + 4 * np.angle(t2)
        cf[49] = -55 * np.abs(np.abs(t1) - np.abs(t2))
        cf[54] = 33 * np.abs(t1)**3 + np.abs(t2)**2
        cf[59] = t1**5 + t2**5 - 29
        cf[64] = -22 * np.real(t1**2) + 22 * np.imag(t2**2)
        cf[69] = (np.sum(range(1, 6)) * np.imag(t1)) + (np.prod(range(1, 6)) * np.real(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_109(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1**2 - 2*t2 + 5
        cf[2] = np.conj(t1) * t2 + 7
        cf[3] = t2**2 - t1 + 11
        cf[4] = np.abs(t1 + t2) + 13
        cf[5] = np.angle(t1) * np.angle(t2) + 17
        cf[6] = t1 * t2 - 19
        cf[7] = t1**3 + t2**3 + 23
        cf[8] = np.sin(t1) + np.cos(t2) + 29
        cf[9] = np.log(np.abs(t1 + t2) + 1) + 31
        cf[10] = t1**2 - t2**2 + 37
        cf[11] = np.conj(t2) * t1 + 41
        cf[12] = np.imag(t1) * np.real(t2) - 43
        cf[13] = t1 * np.conj(t2) + 47
        cf[14] = np.abs(t1 - t2) + 53
        cf[15] = t1**4 - t2**4 + 59
        cf[16] = 61 - 5 * t1 * t2
        cf[17] = 67 + np.abs(t1**2 + t2**2)
        cf[18] = 71 + t1**5 + t2**5
        cf[19] = 73 - np.angle(t1) * np.angle(t2)
        cf[20] = 79 + np.abs(t1**3 + t2**3)
        cf[21] = 83 - t1**6 + t2**6
        cf[22] = 89 + np.sin(t1 + t2)
        cf[23] = np.abs(np.real(t1) * np.imag(t2)) + 97
        cf[24] = 101 + t1 * t2**2
        cf[25] = 103 - np.conj(t1) * np.real(t2)
        cf[26] = 107 + t1**7 - t2**7
        cf[27] = 109 + np.abs(np.conj(t1 - t2))
        cf[28] = 113 - np.abs(t1**2 - t2**2)
        cf[29] = 127 + (t1**8 * t2**8)
        cf[30] = t1 - t2 + np.abs(t1 * t2) + 131
        cf[31] = 137 + np.angle(t1**2) - np.angle(t2**2)
        cf[32] = 139 - t1**9 + t2**9
        cf[33] = np.log(np.abs(t1 * t2) + 1) + 149
        cf[34] = 151 + (np.abs(t1) + np.abs(t2))**2
        cf[35] = np.sin(2 * t1) - np.cos(2 * t2) + 157
        cf[36] = np.log(np.abs(t1 - t2) + 1) + 163
        cf[37] = 167 + np.real(t1**3) - np.imag(t2**3)
        cf[38] = 173 - (t1**2 * t2**2)**1.5
        cf[39] = 179 + np.angle(t1 * t2) + 1j
        cf[40] = 181 - np.conj(t1**3 - t2**3)
        cf[41] = 191 + np.abs(t1) * np.abs(t2)
        cf[42] = 193 - np.abs(np.real(t1) + np.imag(t2))
        cf[43] = 197 + np.sin(t1**2 + t2**2)
        cf[44] = 199 - t1 * t2**3
        cf[45] = t1 * np.imag(t2) + 211
        cf[46] = np.abs(t1**4 + t2**4) + 223
        cf[47] = 227 - np.conj(t1**2) * np.conj(t2**2)
        cf[48] = 229 + np.sin(t1 * t2) - np.cos(t1 - t2)
        cf[49] = 233 + t1**9 - t2**9
        cf[50] = 239 - np.abs(np.conj(t1**2 + t2**2))
        cf[51] = 241 + t1**3 + t2**3
        cf[52] = t1**10 + t2**10 + 251
        cf[53] = t1 * t2 * np.real(t1 + t2) - 257
        cf[54] = np.abs(t1 - t2) - 263
        cf[55] = t1**11 - t2**11 + 269
        cf[56] = 271 + np.abs(t1 * t2**2 - t2**3)
        cf[57] = 277 + np.sin(t1**3 - t2**3)
        cf[58] = 281 - np.conj(t1**2 * t2)
        cf[59] = np.conj(t1**5 + t2**5) + 283
        cf[60] = np.angle(t1**3 * t2**3) + 293
        cf[61] = 307 - np.sin(t1 * t2 + 1j)
        cf[62] = np.abs(t1**6 + t2**6) + 311
        cf[63] = 313 - np.cos(t1**3 - t2**3)
        cf[64] = np.angle(t1 * t2) + 317
        cf[65] = np.real(t1**2 - t2**2) - 331
        cf[66] = 337 + np.abs(t1**6 * t2**6)
        cf[67] = 347 - np.abs(t1**4 - t2**4)
        cf[68] = 349 + np.sin(np.conj(t1 - t2))
        cf[69] = 353 - np.cos(t1 + t2**2)
        cf[70] = np.abs((t1 + t2)**3 - 359)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_110(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        prime_sequence = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59])
        for k in range(1, 36):
            cf[k - 1] = np.real(t1) * prime_sequence[k % len(prime_sequence)] + np.imag(t2) * k**2
            cf[70 - k] = np.real(t2) * prime_sequence[(70 - k) % len(prime_sequence)] - np.imag(t1) * k**2
        cf[35] = np.sum(prime_sequence) * (np.cos(np.angle(t1)) + 1j * np.sin(np.angle(t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_111(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71])
        
        def cyclo(n):
            return np.prod(1 - t1**n * np.exp((np.arange(1, n + 1) - 1) * 2j * np.pi / n))
        
        for r in range(1, 21):
            cf[r - 1] = (primes[r - 1] * (t1**r + t2**r)) / cyclo(r + 1)
        
        for r in range(21, 31):
            cf[r - 1] = cyclo(primes[r - 21]) * (t1 + t2)
        
        for r in range(31, 46):
            theta = np.angle(t1 + t2)
            cf[r - 1] = ((r - 30) * np.abs(t1 - t2) * np.cos(r * theta)) / (1 + np.abs(t1)**r + np.abs(t2)**r)
        
        for r in range(46, 61):
            cf[r - 1] = (np.sin(primes[r - 46] * t1) + np.cos(primes[r - 46] * t2)) * ((-1j) ** (r - 45)) / np.prod(np.arange(1, r + 1))
        
        for r in range(61, 71):
            cf[r - 1] = (np.log(np.abs(t1) + r) * np.real(t2**2 - t1**2)) / (primes[r - 61] * cyclo(r - 50))
        
        cf[70] = np.conj(np.sum(cf[0:35])) + np.prod(cf[35:70])
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_112(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        
        def phi_n(n):
            theta = 2 * np.pi / n
            w = np.exp(1j * theta)
            return w**np.arange(n)
        
        cf[0:10] = phi_n(10) + (t1**2 + np.abs(t2))
        cf[10:15] = np.exp(np.arange(2, 7) * np.angle(t1)) - np.real(t2)
        cf[15:20] = phi_n(5) * (t1 + 2j * t2)
        cf[20:30] = 100 - np.real(t1**3) + 1j * np.imag(t2**2)
        cf[30:40] = -40 + np.abs(t1 * t2) + 1j * np.angle(t1 - t2)
        cf[40:50] = 1j * phi_n(10)**(np.arange(2, 12)) - (t1 + np.conj(t2))
        cf[50:60] = 2 * np.log(np.abs(np.real(t1) + np.imag(t2)) + 1) * np.arange(2, 12)
        cf[60:70] = ((-1)**np.arange(1, 11)) * phi_n(10)**(np.arange(1, 11) * t1)
        cf[70] = np.sum(cf[0:70]) / 71
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_113(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        z = t1 + t2 * 1j
        for k in range(1, 36):
            cf[k - 1] = np.cos(np.pi * k / 35) * ((-1)**k) * np.abs(z)**k
            cf[70 - k] = np.sin(np.pi * (35 - k) / 35) * ((-1)**(k + 1)) * np.angle(z)**(35 - k)
        cf[35] = np.exp(np.abs(z))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_114(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.real(t1**2 + t2**3) - np.imag(t1 * np.conj(t2))
        cf[1:4] = np.array([-827, 221, 653]) * (np.real(t1) + np.imag(t2))
        cf[4] = np.abs(t1 - 2j * t2)**5
        for j in range(6, 29):
            cf[j - 1] = np.cos(j * np.angle(t1 + t2)) * np.sin(j * np.abs(t1**2 + t2)) + j
        cf[28:41] = np.array([89, -233, 144, 377, 610, -987, 1597, -2584, 4181, -6765, 10946, -17711, 28657]) * np.abs(t1 - t2)
        for k in range(42, 62):
            cf[k - 1] = np.log(np.abs(k * t1 * np.conj(t2) + 71))
        cf[62:66] = np.array([3j, 2 - 8j, -6 + 11j, -5.5]) * (t1**3 - t2**3)
        cf[66:71] = np.tan(np.pi / 4) * np.exp(-(np.arange(66, 71)))
        cf[70] = np.exp(t1) - np.exp(t2) 
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_115(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        kappa = (1 + t1**2) * (1 - t2)**2 * np.angle(t1) * np.angle(t2)
        offset = np.real(t1) - np.imag(t2) + 1j * (np.imag(t1) + np.real(t2))
        
        cf[0] = kappa - t1**3 + t2**2
        cf[1] = -2 * offset + t1 * t2 + np.abs(t1 + t2)
        cf[2] = (3 + 2 * kappa) * (t1 - t2)
        cf[3] = 0.5 * (offset - 1j * kappa)
        cf[4:10] = np.abs(t1) / (np.arange(1, 7))
        cf[10:20] = -(t2**2) * (np.arange(1, 11))
        
        for k in range(21, 31):
            cf[k - 1] = (t1**k - t2**(k - 1)) / (k**2)
        
        cf[30:51] = np.real(offset) * (np.arange(21, 41)) + 0.1 * np.imag(offset) * (np.arange(1, 21))
        
        for k in range(51, 61):
            cf[k - 1] = np.imag(t1 * t2)**2 / (k**2)
        
        cf[61] = np.abs(offset) + 0.1 * t2**2 - 0.1 * t1**2
        cf[62] = 0.01 * (t1**3 - 2 * t2**3)
        cf[63] = 0.001 * (offset * np.conj(t2))
        cf[64:70] = ((np.arange(64, 70) + 1) * np.real(offset) + (np.arange(64, 70) + 1) * np.imag(offset)**2) / 2
        cf[70] = -t1 + 2j * t2
        cf[71] = (1 + (t1**3 * np.conj(t2))) / 3
        
        return cf.astype(np.complex128) 
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_116(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47])
        for k in range(1, 36):
            cf[k - 1] = primes[k % 15] * (t1**k + t2**k) * (-1)**k / (k + 1)
            cf[70 - k] = primes[(k + 11) % 15] * (t1**(71 - k) - t2**(71 - k)) * (-1)**(71 - k) / (71 - k + 1)
        cf[35] = np.sum(primes[:5]) * np.abs(t1 + t2) / (1 + np.abs(t1))
        cf[70] = 1 + 1j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_117(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = (t1 + t2)**(k - 1) + ((-1)**k) * (np.exp(1j * k * np.pi / 71)) * (k**(1/3))
        cf *= (1 + np.log(np.abs(cf) + 1) / (1 + np.abs(t1 * t2)))
        cf[0:10] += (t1**2 + t2**2)**(1/3)
        cf[61:71] *= np.exp(-1j * np.angle(t1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_118(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53])
        f1 = t1 * np.sum(primes) + t2
        f2 = t2 * np.sum(primes[:8]) + np.conj(t1)
        cf[0:16] = primes[:16] * (t1 - t2)
        cf[16:32] = f1**2 - f2**2
        cf[32:48] = (t1**3 - t2**3) * (primes[:16] - f1)
        cf[48:64] = (primes[:16] * t1**2 + t2**3) - t1
        cf[64:70] = np.sin(cf[0:6] * t2) + np.cos(cf[0:6] * t1)
        cf[70] = np.prod(primes[:9])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_119(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47])
        cycle = np.array([1, -1, 1j, -1j, 0.5, -0.5, 0.5j, -0.5j])
        for k in range(1, 16):
            cf[k - 1] = primes[k - 1] * (t1 + cycle[k % 8] * t2) / (k + 1)
        for k in range(16, 31):
            cf[k - 1] = np.real(t1)**k - np.imag(t2)**(k % 5 + 1) + cycle[k % 8]
        for k in range(31, 46):
            cf[k - 1] = np.abs(t1 * t2) * np.sin(np.angle(t2)**2) + np.cos(k * t1) * cycle[k % 8]
        for k in range(46, 56):
            cf[k - 1] = 1 / np.abs(t1 + t2) * k**2 + np.sum(primes[:3]) * 1j**k * cycle[k % 8]
        for k in range(56, 66):
            cf[k - 1] = np.log(np.abs(t1 * k + t2 * k + 1)) * cycle[k % 8]
        cf[66] = np.prod(primes[:10]) / (np.abs(t1)**2 + np.abs(t2)**2 + 1)
        cf[67] = np.sum(primes[10:15]) * (t1 - t2)**3
        cf[68] = np.real(t1**2 * t2) - np.imag(t2**2 * t1) + 1j
        cf[69] = (t1 * t2 + 1)**35
        cf[70] = -np.prod(cycle) * (t1 + t2)
        cf[71] = np.abs(t1 * cycle[0]) * np.abs(t2 * cycle[1]) + 1
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_120(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        theta = np.angle(t1) * np.angle(t2)
        mult_factors = (-1)**(np.arange(1, 71) + 1)
        cf[0:10] = (np.arange(1, 11)) * t1**2 - (np.arange(10, 0, -1)) * t2**2
        for k in range(11, 41):
            cf[k - 1] = (k % 2) * np.abs(t1) + (k % 3) * np.abs(t2) * np.exp((k / 5) * theta * 1j)
        cf[40:60] = ((np.arange(41, 61)) + np.log(np.abs(theta) + 1)) * np.conj(t1) * 5 * mult_factors[0:20]
        cf[60:71] = (np.arange(61, 71)) - (np.arange(1, 11)) * t2 - np.sum((np.arange(1, 11)) * mult_factors[10:20])
        cf[70] = (np.sum(np.arange(1, 36)) + np.sum(np.arange(36, 72))) / np.prod(np.arange(1, 16))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_121(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        w = np.exp(2 * np.pi * 1j / 7)
        for k in range(1, 8):
            cf[k - 1] = (t1**k - (t2 / w)**k) * np.real(w**k)
        cf[7] = -np.sum(t1**2, t2**2) + np.real(t1 * t2) + np.imag(t1 * t2)
        for k in range(9, 36):
            z = np.angle(t1) + np.angle(t2)
            cf[k - 1] = np.cos(k * z) + 1j * np.sin(k * z)
        for k in range(36, 71):
            cf[k - 1] = (np.abs(t1) * t2 + t1 * np.imag(t2))**2 / (k + 1)
        cf[70] = np.abs(t1) - np.abs(t2) + np.log(np.abs(t1 + t2 + 1) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_122(t1, t2):
    try:
        if not 'n' in ps.poly or ps.poly['n'] is None:
            n = int(71)
            m = int(36)
        else:
            n = int(ps.poly['n'])
            m = int(n/2)+1
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, m):
            cf[k - 1] = (-1)**k * (k**2 + t1 * t2**k + k * np.abs(t1)) * (np.cos(k * np.angle(t2)))
        for k in range(m, n):
            cf[k - 1] = (k**3 + t2 * t1**k + k * np.abs(t2)) * (np.sin(k * np.angle(t1)))
        cf[n-1] = np.sum(np.abs(cf[0:(n-1)]))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_123_old(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.real(t1)**2 - np.imag(t2)**2
        cf[1] = (t1 + t2)**2 - 7
        cf[2] = (t1**2 - t2**2)
        cf[3:10] = np.arange(3, 30, 4) * np.abs(t1 + 1j * t2)
        cf[10:20] = np.real(t1 - t2) * np.arange(11, 21)
        cf[20:30] = 1 / (1 + np.arange(21, 31)) * np.real(t1 + t2)
        cf[30] = np.angle(t1) * np.imag(t2)
        cf[31:51] = 1000 * (-1)**np.arange(32, 51) * t1 * t2
        cf[51:61] = 2000 * (-1)**np.arange(51, 61) * np.log(np.abs(t1) + 1)
        cf[61:66] = 1j * np.conj(t1 * t2) * np.sqrt(np.arange(61, 66))
        cf[66:71] = (np.arange(66, 71) * (np.arange(66, 71) - 1)) / (np.abs(t1) + np.abs(t2) + 1)
        cf[70] = np.prod(np.arange(1, 6))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_123(t1, t2):
    cf = np.zeros(71, dtype=np.complex128)
    cf[0] = t1.real**2 - t2.imag**2
    cf[1] = (t1 + t2)**2 - 7
    cf[2] = t1**2 - t2**2
    cf[3:10] = np.arange(3, 30, 4) * np.abs(t1 + 1j * t2)
    cf[10:20] = (t1 - t2).real * np.arange(11, 21)
    cf[20:30] = 1 / (1 + np.arange(21, 31)) * (t1 + t2).real
    cf[30] = np.angle(t1) * t2.imag
    cf[31:50] = 1000 * (-1)**np.arange(32, 51) * t1 * t2
    cf[50:60] = 2000 * (-1)**np.arange(51, 61) * np.log(np.abs(t1) + 1)
    cf[60:65] = 1j * np.conj(t1 * t2) * np.sqrt(np.arange(61, 66))
    cf[65:70] = np.arange(66, 71) * (np.arange(66, 71) - 1) / (np.abs(t1) + np.abs(t2) + 1)
    cf[70] = np.prod(np.arange(1, 6))
    return cf.astype(np.complex128)


def poly_124(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61])

        for j in range(1, 36):
            cf[j - 1] = primes[j % len(primes)] * (t1**j - t2**(71 - j))

        for k in range(36, 72):
            cf[k - 1] = (np.abs(t1) * np.real(t2) - np.imag(t1) * np.angle(t2))**(142 - k) / (1 + np.abs(primes[k % len(primes)]))

        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_125(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:15] = [(-1)**j * (j**2) * (np.abs(t1) + np.abs(t2)) for j in range(1, 16)]
        cf[15:30] = [(-1)**(k + 1) * (k**3) * np.angle(t1 + 1j * t2) for k in range(16, 31)]
        cf[30:45] = [(-1)**(r + 1) * np.cos(r * t1) + np.sin(r * t2) for r in range(31, 46)]
        for s in range(46, 61):
            cf[s] = (-1)**s * (s**2) * np.conj(t1) * np.conj(t2)
        cf[61:70] = [n**3 * np.log(np.abs(t1 * t2) + 1) for n in range(61, 71)]
        cf[70] = 1
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_126(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(1, 36):
            cf[j - 1] = ((-1)**j) * (t1**3 + t2**2) * j**2
            cf[70 - j] = ((-1)**(j + 1)) * (t2**3 + t1**2) * j**1.5
        middle_index = 36
        for k in range(1, 6):
            cf[middle_index + k - 1] = np.exp(1j * np.pi * ((-1)**k) * (t1 + t2) / 2)
        cf[middle_index - 1] = np.log(np.abs(t1 + t2) + 1)
        cf[0] = np.sin(t1**3) + np.cos(t2**3)
        cf[70] = np.cos(t1) * np.sin(t2) + t1**2 + t2**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_127(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(0, 71):
            cf[k] = (t1**(70 - k) + np.conj(t2)**k) * (-1)**k * np.log(np.abs(t1 + t2) + k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_128(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        k = np.arange(1, 72)
        cf[:] = (-1)**k * (t1**k + np.conj(t2)**(71 - k)) * (72 - k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_129(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + 2 * t2**3
        cf[1] = np.conj(t1) * t2**2 - t1**3
        cf[2] = t1**4 - t2**4 + t1 * t2
        cf[3] = t1**2 * t2 + 3 * t1
        cf[4] = 4 * t2**5 - t1 * t2**2
        cf[5] = t1**3 - 5 * t2**6
        cf[6] = 6 * t1 * t2**3 + t2
        cf[7] = t1**7 - 7 * t2**7
        cf[8] = 8 * t1**2 * t2**4 - t1
        cf[9] = t1**8 + 9 * t2**8
        cf[10] = 10 * t1 * t2**5 - t1**2
        cf[11] = t1**9 - 11 * t2**9
        cf[12] = 12 * t1**2 * t2**6 + t2
        cf[13] = t1**10 - 13 * t2**10
        cf[14] = 14 * t1 * t2**7 - t1**3
        cf[15] = t1**11 + 15 * t2**11
        cf[16] = 16 * t1**2 * t2**8 - t2
        cf[17] = t1**12 - 17 * t2**12
        cf[18] = 18 * t1 * t2**9 + t1
        cf[19] = t1**13 - 19 * t2**13
        cf[20] = 20 * t1**2 * t2**10 - t2**2
        cf[21] = t1**14 + 21 * t2**14
        cf[22] = 22 * t1 * t2**11 - t1**4
        cf[23] = t1**15 - 23 * t2**15
        cf[24] = 24 * t1**2 * t2**12 + t2**3
        cf[25] = t1**16 + 25 * t2**16
        cf[26] = 26 * t1 * t2**13 - t1**5
        cf[27] = t1**17 - 27 * t2**17
        cf[28] = 28 * t1**2 * t2**14 + t2**4
        cf[29] = t1**18 + 29 * t2**18
        cf[30] = 30 * t1 * t2**15 - t1**6
        cf[31] = t1**19 - 31 * t2**19
        cf[32] = 32 * t1**2 * t2**16 + t2**5
        cf[33] = t1**20 + 33 * t2**20
        cf[34] = 34 * t1 * t2**17 - t1**7
        cf[35] = t1**21 - 35 * t2**21
        cf[36] = 36 * t1**2 * t2**18 + t2**6
        cf[37] = t1**22 + 37 * t2**22
        cf[38] = 38 * t1 * t2**19 - t1**8
        cf[39] = t1**23 - 39 * t2**23
        cf[40] = 40 * t1**2 * t2**20 + t2**7
        cf[41] = t1**24 + 41 * t2**24
        cf[42] = 42 * t1 * t2**21 - t1**9
        cf[43] = t1**25 - 43 * t2**25
        cf[44] = 44 * t1**2 * t2**22 + t2**8
        cf[45] = t1**26 + 45 * t2**26
        cf[46] = 46 * t1 * t2**23 - t1**10
        cf[47] = t1**27 - 47 * t2**27
        cf[48] = 48 * t1**2 * t2**24 + t2**9
        cf[49] = t1**28 + 49 * t2**28
        cf[50] = 50 * t1 * t2**25 - t1**11
        cf[51] = t1**29 - 51 * t2**29
        cf[52] = 52 * t1**2 * t2**26 + t2**10
        cf[53] = t1**30 + 53 * t2**30
        cf[54] = 54 * t1 * t2**27 - t1**12
        cf[55] = t1**31 - 55 * t2**31
        cf[56] = 56 * t1**2 * t2**28 + t2**11
        cf[57] = t1**32 + 57 * t2**32
        cf[58] = 58 * t1 * t2**29 - t1**13
        cf[59] = t1**33 - 59 * t2**33
        cf[60] = 60 * t1**2 * t2**30 + t2**12
        cf[61] = t1**34 + 61 * t2**34
        cf[62] = 62 * t1 * t2**31 - t1**14
        cf[63] = t1**35 - 63 * t2**35
        cf[64] = 64 * t1**2 * t2**32 + t2**13
        cf[65] = t1**36 + 65 * t2**36
        cf[66] = 66 * t1 * t2**33 - t1**15
        cf[67] = t1**37 - 67 * t2**37
        cf[68] = 68 * t1**2 * t2**34 + t2**14
        cf[69] = t1**38 + 69 * t2**38
        cf[70] = np.log(np.abs(t1) + 1) + np.real(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_130(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**10 + 2 * t2**8 + t1 * t2
        cf[1] = 3 * t1**9 - t2**7 + t1**2 * t2
        cf[2] = -2 * t1**8 + 4 * t2**6 + t1 * t2**2
        cf[3] = t1**7 - 3 * t2**5 + 2 * t1**3 * t2
        cf[4] = 5 * t1**6 + t2**4 - t1**2 * t2**3
        cf[5] = -t1**5 + 6 * t2**3 + t1**4 * t2
        cf[6] = 4 * t1**4 - 2 * t2**2 + t1 * t2**4
        cf[7] = -3 * t1**3 + 5 * t2 + t1**3 * t2**2
        cf[8] = 2 * t1**2 - 4 * t2**5 + t1 * t2**5
        cf[9] = -t1 + 3 * t2**4 + t1**2 * t2**2
        cf[10] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[11] = 4 * t1**4 * t2 + 2 * t2**2 - t1 * t2**4
        cf[12] = -2 * t1**3 * t2**2 + 3 * t2 + t1**3 * t2
        cf[13] = 3 * t1**2 * t2**3 - t2**4 + t1 * t2**2
        cf[14] = -t1 * t2**4 + 2 * t2**2 + t1**2 * t2
        cf[15] = t1**6 + t2**7 - t1 * t2
        cf[16] = -t1**5 + 2 * t2**6 + t1**3 * t2
        cf[17] = 3 * t1**4 - t2**5 + t1**2 * t2**2
        cf[18] = -2 * t1**3 + 4 * t2**4 + t1 * t2**3
        cf[19] = 5 * t1**2 - t2**3 + t1 * t2**4
        cf[20] = -t1 + 3 * t2**2 + t1**2 * t2
        cf[21] = t1 * t2**5 - t2 + t1**3 * t2
        cf[22] = 2 * t1**4 * t2 - t2**2 + t1 * t2**5
        cf[23] = -3 * t1**3 * t2**2 + 4 * t2 + t1**2 * t2**3
        cf[24] = 4 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[25] = -2 * t1 * t2**4 + 3 * t2**2 + t1**2 * t2**2
        cf[26] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[27] = 3 * t1**4 * t2 + 2 * t2**2 - t1 * t2**4
        cf[28] = -t1**3 * t2**2 + 5 * t2 + t1 * t2**3
        cf[29] = 2 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[30] = -t1 * t2**4 + 4 * t2**2 + t1**2 * t2**2
        cf[31] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[32] = 4 * t1**4 * t2 + 3 * t2**2 - t1 * t2**4
        cf[33] = -t1**3 * t2**2 + 6 * t2 + t1 * t2**3
        cf[34] = 3 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[35] = -2 * t1 * t2**4 + 5 * t2**2 + t1**2 * t2**2
        cf[36] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[37] = 5 * t1**4 * t2 + 4 * t2**2 - t1 * t2**4
        cf[38] = -t1**3 * t2**2 + 7 * t2 + t1 * t2**3
        cf[39] = 4 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[40] = -3 * t1 * t2**4 + 6 * t2**2 + t1**2 * t2**2
        cf[41] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[42] = 6 * t1**4 * t2 + 5 * t2**2 - t1 * t2**4
        cf[43] = -t1**3 * t2**2 + 8 * t2 + t1 * t2**3
        cf[44] = 5 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[45] = -4 * t1 * t2**4 + 7 * t2**2 + t1**2 * t2**2
        cf[46] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[47] = 7 * t1**4 * t2 + 6 * t2**2 - t1 * t2**4
        cf[48] = -t1**3 * t2**2 + 9 * t2 + t1 * t2**3
        cf[49] = 6 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[50] = -5 * t1 * t2**4 + 8 * t2**2 + t1**2 * t2**2
        cf[51] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[52] = 8 * t1**4 * t2 + 7 * t2**2 - t1 * t2**4
        cf[53] = -t1**3 * t2**2 + 10 * t2 + t1 * t2**3
        cf[54] = 7 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[55] = -6 * t1 * t2**4 + 9 * t2**2 + t1**2 * t2**2
        cf[56] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[57] = 9 * t1**4 * t2 + 8 * t2**2 - t1 * t2**4
        cf[58] = -t1**3 * t2**2 + 11 * t2 + t1 * t2**3
        cf[59] = 8 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[60] = -7 * t1 * t2**4 + 10 * t2**2 + t1**2 * t2**2
        cf[61] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[62] = 10 * t1**4 * t2 + 9 * t2**2 - t1 * t2**4
        cf[63] = -t1**3 * t2**2 + 12 * t2 + t1 * t2**3
        cf[64] = 9 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[65] = -8 * t1 * t2**4 + 11 * t2**2 + t1**2 * t2**2
        cf[66] = t1**5 * t2 - t2**3 + t1 * t2**3
        cf[67] = 11 * t1**4 * t2 + 10 * t2**2 - t1 * t2**4
        cf[68] = -t1**3 * t2**2 + 13 * t2 + t1 * t2**3
        cf[69] = 10 * t1**2 * t2**3 - t2**4 + t1 * t2**4
        cf[70] = -9 * t1 * t2**4 + 12 * t2**2 + t1**2 * t2**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_131(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = (-1)**k * (np.real(t1)**k + np.imag(t2)**k) + (np.cos(k * t1) + np.sin(k * t2)) / k
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_132(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**10 - t2**8 + np.real(t1 * t2)
        cf[1] = -t1**9 + t2**7 - np.imag(t1 + t2)
        cf[2] = t1**8 - t2**6 + np.real(t1**2 * t2)
        cf[3] = -t1**7 + t2**5 - np.real(t1 * t2**2)
        cf[4] = t1**6 - t2**4 + np.real(t1**3)
        cf[5] = -t1**5 + t2**3 - np.real(t2**3)
        cf[6] = t1**4 - t2**2 + np.real(t1**2 * t2)
        cf[7] = -t1**3 + t2 - np.real(t1 * t2)
        cf[8] = t1**2 - np.real(t2**2)
        cf[9] = -t1 + np.real(t1 * t2)
        for k in range(11, 61):
            cf[k - 1] = ((-1)**k * (np.real(t1) + np.imag(t2))**k) / (k + 1)
        for k in range(61, 71):
            cf[k - 1] = ((-1)**k * (np.real(t2) - np.imag(t1))**(k - 60)) / (k**2)
        cf[70] = np.real(t1 * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_133(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k - 1] = (t1**k * np.sin(t2 * k) + t2**k * np.cos(t1 * k)) * (-1)**k / (k + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_134(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k-1] = (np.sin(t1 * k) + np.cos(np.conj(t2) * k)) * (-1)**k / (k + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_135(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 71):
            cf[k-1] = (t1**k * np.sin(k * t2) + (-1)**k * t2**(k-1) * np.cos(k * t1)) / k
        cf[70] = (np.log(np.abs(t1) + np.abs(t2) + 1) + np.sin(t1 * t2)) / 71
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_136(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + 2*t2**3 + 1
        cf[1] = t1*t2 - 3
        cf[2] = np.real(t1) * np.imag(t2) - np.real(t2) * np.imag(t1)
        cf[3] = np.sin(t1) + np.cos(t2)
        cf[4] = t1**2 - t2**2 + t1*t2
        for j in range(6, 36):
            cf[j-1] = ((t1 + (-1)**j * t2)**j) / j
        for k in range(36, 71):
            cf[k-1] = ((t1 - t2)**k) / (k**2) * (-1)**k
        cf[70] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1) + t1*t2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_137(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + 2*t2**3 + np.real(t1*t2)
        cf[1] = t1**4 - 3*t2**2 + np.imag(t1 + t2)
        cf[2] = 4*t1**3 + 2*t2**4 - np.sin(t1)
        cf[3] = 5*t1**2 - 2*t2**5 + np.cos(t2)
        cf[4] = 6*t1 - 3*t2**6 + np.real(t1**2 * t2)
        cf[5] = 7 + 4*t2**7 - np.imag(t1 * t2**2)
        cf[6] = 8*t1**2 + 5*t2**8 + np.real(t1**3)
        cf[7] = 9*t1**3 - 6*t2**9 + np.imag(t2**3)
        cf[8] = 10*t1**4 + 7*t2**10 + np.real(t1*t2**3)
        cf[9] = 11*t1**5 - 8*t2**11 + np.imag(t1**2 * t2**2)
        cf[10] = 12*t1**6 + 9*t2**12 + np.real(t2**4)
        cf[11] = 13*t1**7 - 10*t2**13 + np.imag(t1**3 * t2)
        cf[12] = 14*t1**8 + 11*t2**14 + np.real(t1**4)
        cf[13] = 15*t1**9 - 12*t2**15 + np.imag(t2**5)
        cf[14] = 16*t1**10 + 13*t2**16 + np.real(t1*t2**4)
        cf[15] = 17*t1**11 - 14*t2**17 + np.imag(t1**2 * t2**3)
        cf[16] = 18*t1**12 + 15*t2**18 + np.real(t2**6)
        cf[17] = 19*t1**13 - 16*t2**19 + np.imag(t1**3 * t2**2)
        cf[18] = 20*t1**14 + 17*t2**20 + np.real(t1**4 * t2)
        cf[19] = 21*t1**15 - 18*t2**21 + np.imag(t2**7)
        cf[20] = 22*t1**16 + 19*t2**22 + np.real(t1*t2**5)
        cf[21] = 23*t1**17 - 20*t2**23 + np.imag(t1**2 * t2**4)
        cf[22] = 24*t1**18 + 21*t2**24 + np.real(t2**8)
        cf[23] = 25*t1**19 - 22*t2**25 + np.imag(t1**3 * t2**3)
        cf[24] = 26*t1**20 + 23*t2**26 + np.real(t1**4 * t2**2)
        cf[25] = 27*t1**21 - 24*t2**27 + np.imag(t2**9)
        cf[26] = 28*t1**22 + 25*t2**28 + np.real(t1*t2**6)
        cf[27] = 29*t1**23 - 26*t2**29 + np.imag(t1**2 * t2**5)
        cf[28] = 30*t1**24 + 27*t2**30 + np.real(t2**10)
        cf[29] = 31*t1**25 - 28*t2**31 + np.imag(t1**3 * t2**4)
        cf[30] = 32*t1**26 + 29*t2**32 + np.real(t1**4 * t2**3)
        cf[31] = 33*t1**27 - 30*t2**33 + np.imag(t2**11)
        cf[32] = 34*t1**28 + 31*t2**34 + np.real(t1*t2**7)
        cf[33] = 35*t1**29 - 32*t2**35 + np.imag(t1**2 * t2**6)
        cf[34] = 36*t1**30 + 33*t2**36 + np.real(t2**12)
        cf[35] = 37*t1**31 - 34*t2**37 + np.imag(t1**3 * t2**5)
        cf[36] = 38*t1**32 + 35*t2**38 + np.real(t1**4 * t2**4)
        cf[37] = 39*t1**33 - 36*t2**39 + np.imag(t2**13)
        cf[38] = 40*t1**34 + 37*t2**40 + np.real(t1*t2**8)
        cf[39] = 41*t1**35 - 38*t2**41 + np.imag(t1**2 * t2**7)
        cf[40] = 42*t1**36 + 39*t2**42 + np.real(t2**14)
        cf[41] = 43*t1**37 - 40*t2**43 + np.imag(t1**3 * t2**6)
        cf[42] = 44*t1**38 + 41*t2**44 + np.real(t1**4 * t2**5)
        cf[43] = 45*t1**39 - 42*t2**45 + np.imag(t2**15)
        cf[44] = 46*t1**40 + 43*t2**46 + np.real(t1*t2**9)
        cf[45] = 47*t1**41 - 44*t2**47 + np.imag(t1**2 * t2**8)
        cf[46] = 48*t1**42 + 45*t2**48 + np.real(t2**16)
        cf[47] = 49*t1**43 - 46*t2**49 + np.imag(t1**3 * t2**7)
        cf[48] = 50*t1**44 + 47*t2**50 + np.real(t1**4 * t2**6)
        cf[49] = 51*t1**45 - 48*t2**51 + np.imag(t2**17)
        cf[50] = 52*t1**46 + 49*t2**52 + np.real(t1*t2**10)
        cf[51] = 53*t1**47 - 50*t2**53 + np.imag(t1**2 * t2**9)
        cf[52] = 54*t1**48 + 51*t2**54 + np.real(t2**18)
        cf[53] = 55*t1**49 - 52*t2**55 + np.imag(t1**3 * t2**8)
        cf[54] = 56*t1**50 + 53*t2**56 + np.real(t1**4 * t2**7)
        cf[55] = 57*t1**51 - 54*t2**57 + np.imag(t2**19)
        cf[56] = 58*t1**52 + 55*t2**58 + np.real(t1*t2**11)
        cf[57] = 59*t1**53 - 56*t2**59 + np.imag(t1**2 * t2**10)
        cf[58] = 60*t1**54 + 57*t2**60 + np.real(t2**20)
        cf[59] = 61*t1**55 - 58*t2**61 + np.imag(t1**3 * t2**9)
        cf[60] = 62*t1**56 + 59*t2**62 + np.real(t1**4 * t2**8)
        cf[61] = 63*t1**57 - 60*t2**63 + np.imag(t2**21)
        cf[62] = 64*t1**58 + 61*t2**64 + np.real(t1*t2**12)
        cf[63] = 65*t1**59 - 62*t2**65 + np.imag(t1**2 * t2**11)
        cf[64] = 66*t1**60 + 63*t2**66 + np.real(t2**22)
        cf[65] = 67*t1**61 - 64*t2**67 + np.imag(t1**3 * t2**10)
        cf[66] = 68*t1**62 + 65*t2**68 + np.real(t1**4 * t2**9)
        cf[67] = 69*t1**63 - 66*t2**69 + np.imag(t2**23)
        cf[68] = 70*t1**64 + 67*t2**70 + np.real(t1*t2**13)
        cf[69] = 71*t1**65 - 68*t2**71 + np.imag(t1**2 * t2**12)
        cf[70] = np.log(np.abs(t1) + np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_138(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(2, 72):
            if k % 2 == 0:
                cf[k-1] = (t1**k - t2**k) * (-1)**k / np.log(k + np.abs(t1) + 1)
            else:
                cf[k-1] = (t1**(k//2) + t2**(k//3)) * (1 + np.sin(k * np.angle(t1 + t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_139(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**6 + 2*t2**5
        cf[1] = -3*t1**5 + 4*t2**4
        cf[2] = 5*t1**4 - 6*t2**3
        cf[3] = -7*t1**3 + 8*t2**2
        cf[4] = 9*t1**2 - 10*t2
        cf[5] = -11*t1 + 12
        for k in range(7, 71):
            cf[k-1] = (-1)**k * (t1**(k-5) + t2**(k-3)) / (k-6)
        cf[70] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_140(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 - 2*t2 + 1
        cf[1] = 3*t1**4 + np.conj(t2)**2
        cf[2] = -4*t1**3 + 5*t1*t2
        cf[3] = 6*t2**3 - 7*t1**2*t2
        cf[4] = 8*t1*t2**2 + 9*t1
        cf[5] = -10*t2 + 11*t1**3
        cf[6] = 12*t1**2 - 13*t2**2
        cf[7] = 14*t1*t2 - 15
        cf[8] = -16*t1 + 17*t2**3
        cf[9] = 18*t1**4 - 19*t2
        cf[10] = 20*t1*t2**2 + 21*t1**2
        cf[11] = -22*t2**3 + 23*t1*t2
        cf[12] = 24*t1**3 - 25*t2**2
        cf[13] = 26*t1*t2 - 27
        cf[14] = -28*t1 + 29*t2**3
        cf[15] = 30*t1**4 - 31*t2
        cf[16] = 32*t1*t2**2 + 33*t1**2
        cf[17] = -34*t2**3 + 35*t1*t2
        cf[18] = 36*t1**3 - 37*t2**2
        cf[19] = 38*t1*t2 - 39
        cf[20] = -40*t1 + 41*t2**3
        cf[21] = 42*t1**4 - 43*t2
        cf[22] = 44*t1*t2**2 + 45*t1**2
        cf[23] = -46*t2**3 + 47*t1*t2
        cf[24] = 48*t1**3 - 49*t2**2
        cf[25] = 50*t1*t2 - 51
        cf[26] = -52*t1 + 53*t2**3
        cf[27] = 54*t1**4 - 55*t2
        cf[28] = 56*t1*t2**2 + 57*t1**2
        cf[29] = -58*t2**3 + 59*t1*t2
        cf[30] = 60*t1**3 - 61*t2**2
        cf[31] = 62*t1*t2 - 63
        cf[32] = -64*t1 + 65*t2**3
        cf[33] = 66*t1**4 - 67*t2
        cf[34] = 68*t1*t2**2 + 69*t1**2
        cf[35] = -70*t2**3 + 71*t1*t2
        cf[36] = 72*t1**3 - 73*t2**2
        cf[37] = 74*t1*t2 - 75
        cf[38] = -76*t1 + 77*t2**3
        cf[39] = 78*t1**4 - 79*t2
        cf[40] = 80*t1*t2**2 + 81*t1**2
        cf[41] = -82*t2**3 + 83*t1*t2
        cf[42] = 84*t1**3 - 85*t2**2
        cf[43] = 86*t1*t2 - 87
        cf[44] = -88*t1 + 89*t2**3
        cf[45] = 90*t1**4 - 91*t2
        cf[46] = 92*t1*t2**2 + 93*t1**2
        cf[47] = -94*t2**3 + 95*t1*t2
        cf[48] = 96*t1**3 - 97*t2**2
        cf[49] = 98*t1*t2 - 99
        cf[50] = -100*t1 + 101*t2**3
        cf[51] = 102*t1**4 - 103*t2
        cf[52] = 104*t1*t2**2 + 105*t1**2
        cf[53] = -106*t2**3 + 107*t1*t2
        cf[54] = 108*t1**3 - 109*t2**2
        cf[55] = 110*t1*t2 - 111
        cf[56] = -112*t1 + 113*t2**3
        cf[57] = 114*t1**4 - 115*t2
        cf[58] = 116*t1*t2**2 + 117*t1**2
        cf[59] = -118*t2**3 + 119*t1*t2
        cf[60] = 120*t1**3 - 121*t2**2
        cf[61] = 122*t1*t2 - 123
        cf[62] = -124*t1 + 125*t2**3
        cf[63] = 126*t1**4 - 127*t2
        cf[64] = 128*t1*t2**2 + 129*t1**2
        cf[65] = -130*t2**3 + 131*t1*t2
        cf[66] = 132*t1**3 - 133*t2**2
        cf[67] = 134*t1*t2 - 135
        cf[68] = -136*t1 + 137*t2**3
        cf[69] = 138*t1**4 - 139*t2
        cf[70] = 140*t1*t2**2 + 141*t1**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_141(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + 2*t2
        cf[1] = t1**4 - 3*t2**2 + np.conj(t1)
        cf[2] = 4*t1*t2 + 5*np.sin(t1)
        cf[3] = 6*np.cos(t2) - 7*t1**2
        cf[4] = 8*t2**3 + 9*t1*t2
        cf[5] = 10*np.sin(t1 + t2) - 11*t2
        cf[6] = 12*np.cos(t1) + 13*t1*t2
        cf[7] = 14*t1**3 - 15*t2**2
        cf[8] = 16*t1*t2**2 + 17*np.sin(t2)
        cf[9] = 18*np.cos(t1 + t2) - 19*t1
        cf[10] = 20*t2**4 + 21*t1**2*t2
        cf[11] = 22*np.sin(t1) - 23*t2**3
        cf[12] = 24*np.cos(t2) + 25*t1*t2
        cf[13] = 26*t1**4 - 27*t2**2*t1
        cf[14] = 28*np.sin(t1 + t2) + 29*t2
        cf[15] = 30*np.cos(t1) - 31*t1*t2**2
        cf[16] = 32*t2**5 + 33*t1**3
        cf[17] = 34*np.sin(t2) - 35*t1*t2
        cf[18] = 36*np.cos(t1 + t2) + 37*t2**3
        cf[19] = 38*t1**5 - 39*t2**2
        cf[20] = 40*np.sin(t1) + 41*t1**2*t2
        cf[21] = 42*np.cos(t2) - 43*t2**4
        cf[22] = 44*t1**3*t2 + 45*np.sin(t1 + t2)
        cf[23] = 46*np.cos(t1) - 47*t2**3*t1
        cf[24] = 48*t2**6 + 49*t1**4
        cf[25] = 50*np.sin(t2) + 51*t1*t2**2
        cf[26] = 52*np.cos(t1 + t2) - 53*t2**4
        cf[27] = 54*t1**6 - 55*t2**3
        cf[28] = 56*np.sin(t1) + 57*t1**3*t2
        cf[29] = 58*np.cos(t2) - 59*t2**5
        cf[30] = 60*t1**4*t2 + 61*np.sin(t1 + t2)
        cf[31] = 62*np.cos(t1) + 63*t2**4*t1
        cf[32] = 64*t2**7 + 65*t1**5
        cf[33] = 66*np.sin(t2) - 67*t1*t2**3
        cf[34] = 68*np.cos(t1 + t2) + 69*t2**5
        cf[35] = 70*t1**7 - 71*t2**4
        cf[36] = 72*np.sin(t1) + 73*t1**4*t2
        cf[37] = 74*np.cos(t2) - 75*t2**6
        cf[38] = 76*t1**5*t2 + 77*np.sin(t1 + t2)
        cf[39] = 78*np.cos(t1) - 79*t2**5*t1
        cf[40] = 80*t2**8 + 81*t1**6
        cf[41] = 82*np.sin(t2) + 83*t1*t2**4
        cf[42] = 84*np.cos(t1 + t2) - 85*t2**6
        cf[43] = 86*t1**8 - 87*t2**5
        cf[44] = 88*np.sin(t1) + 89*t1**5*t2
        cf[45] = 90*np.cos(t2) - 91*t2**7
        cf[46] = 92*t1**6*t2 + 93*np.sin(t1 + t2)
        cf[47] = 94*np.cos(t1) + 95*t2**6*t1
        cf[48] = 96*t2**9 + 97*t1**7
        cf[49] = 98*np.sin(t2) - 99*t1*t2**5
        cf[50] = 100*np.cos(t1 + t2) + 101*t2**7
        cf[51] = 102*t1**9 - 103*t2**6
        cf[52] = 104*np.sin(t1) + 105*t1**6*t2
        cf[53] = 106*np.cos(t2) - 107*t2**8
        cf[54] = 108*t1**7*t2 + 109*np.sin(t1 + t2)
        cf[55] = 110*np.cos(t1) + 111*t2**7*t1
        cf[56] = 112*t2**10 + 113*t1**8
        cf[57] = 114*np.sin(t2) - 115*t1*t2**6
        cf[58] = 116*np.cos(t1 + t2) + 117*t2**8
        cf[59] = 118*t1**10 - 119*t2**7
        cf[60] = 120*np.sin(t1) + 121*t1**7*t2
        cf[61] = 122*np.cos(t2) - 123*t2**9
        cf[62] = 124*t1**8*t2 + 125*np.sin(t1 + t2)
        cf[63] = 126*np.cos(t1) + 127*t2**8*t1
        cf[64] = 128*t2**11 + 129*t1**9
        cf[65] = 130*np.sin(t2) - 131*t1*t2**7
        cf[66] = 132*np.cos(t1 + t2) + 133*t2**9
        cf[67] = 134*t1**11 - 135*t2**8
        cf[68] = 136*np.sin(t1) + 137*t1**8*t2
        cf[69] = 138*np.cos(t2) - 139*t2**10
        cf[70] = 140*t1**9*t2 + 141*np.sin(t1 + t2) / 200
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_142(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + 2*t2
        cf[1] = t1**2 - t2
        cf[2] = np.conj(t1) + t2**3
        for k in range(4, 36):
            cf[k-1] = (t1**k + t2**(k-1)) / (k * np.sin(k))
            cf[72 - k - 1] = (t2**k - t1**(k-1)) / (k * np.cos(k))
        for k in range(36, 71):
            cf[k-1] = np.log(np.abs(t1) + 1) * np.sin(k * t2) + np.cos(k * t1)
        cf[70] = (t1**5 + t2**5) / (1 + np.abs(t1*t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_143(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**4 + 2*t2**3 - 3*t1*t2
        cf[1] = 4*t1**3 - 5*t2**2 + 6*np.real(t1)
        cf[2] = -7*t1**2*t2 + 8*np.imag(t2)
        cf[3] = 9*t1*t2**2 - 10*t1**3
        cf[4] = 11*np.abs(t1) + 12*np.angle(t2)
        cf[5] = -13*t1**4 + 14*t2 - 15*np.cos(t1)
        cf[6] = 16*np.sin(t2) + 17*t1*t2**2
        cf[7] = 18*t1**2 - 19*t2**3 + 20*t1
        cf[8] = -21*t1*t2 + 22*np.log(np.abs(t1) + 1) 
        cf[9] = 23*t2**2 - 24*t1**3 + 25*t1*t2
        cf[10] = 26*np.cos(t1*t2) + 27*np.sin(t2)
        cf[11] = -28*t1**2 + 29*t2 - 30*t1*t2**2
        cf[12] = 31*t1**4 - 32*t2**3 + 33*t1**2*t2
        cf[13] = 34*t1*t2 - 35*np.log(np.abs(t2) + 1)
        cf[14] = -36*t1**3 + 37*t2**2 - 38*t1*t2
        cf[15] = 39*np.sin(t1) + 40*np.cos(t2)
        cf[16] = 41*t1**2*t2 - 42*t2**3 + 43*t1
        cf[17] = -44*t1*t2 + 45*np.real(t2)
        cf[18] = 46*t2**2 - 47*t1**4 + 48*t1*t2
        cf[19] = 49*np.imag(t1) + 50*t2**3 - 51*t1*t2**2
        cf[20] = -52*t1**3 + 53*t2 - 54*np.cos(t1*t2)
        cf[21] = 55*t1*t2 + 56*np.log(np.abs(t1) + 1)
        cf[22] = 57*t1**2 - 58*t2**2 + 59*t1*t2
        cf[23] = -60*np.sin(t2) + 61*np.cos(t1)
        cf[24] = 62*t1**4 - 63*t2**3 + 64*t1**2*t2
        cf[25] = 65*t1*t2 - 66*np.real(t1)
        cf[26] = -67*t1**3 + 68*t2**2 - 69*t1*t2
        cf[27] = 70*np.sin(t1) + 71*np.cos(t2)
        cf[28] = 72*t1**2*t2 - 73*t2**3 + 74*t1
        cf[29] = -75*t1*t2 + 76*np.imag(t2)
        cf[30] = 77*t2**2 - 78*t1**4 + 79*t1*t2
        cf[31] = 80*np.real(t1) + 81*t2**3 - 82*t1*t2**2
        cf[32] = -83*t1**3 + 84*t2 - 85*np.sin(t1*t2)
        cf[33] = 86*t1*t2 + 87*np.log(np.abs(t2) + 1)
        cf[34] = 88*t1**2 - 89*t2**2 + 90*t1*t2
        cf[35] = -91*np.cos(t2) + 92*np.sin(t1)
        cf[36] = 93*t1**4 - 94*t2**3 + 95*t1**2*t2
        cf[37] = 96*t1*t2 - 97*np.real(t1)
        cf[38] = -98*t1**3 + 99*t2**2 - 100*t1*t2
        cf[39] = 101*np.sin(t1) + 102*np.cos(t2)
        cf[40] = 103*t1**2*t2 - 104*t2**3 + 105*t1
        cf[41] = -106*t1*t2 + 107*np.imag(t2)
        cf[42] = 108*t2**2 - 109*t1**4 + 110*t1*t2
        cf[43] = 111*np.real(t1) + 112*t2**3 - 113*t1*t2**2
        cf[44] = -114*t1**3 + 115*t2 - 116*np.sin(t1*t2)
        cf[45] = 117*t1*t2 + 118*np.log(np.abs(t1) + 1)
        cf[46] = 119*t1**2 - 120*t2**2 + 121*t1*t2
        cf[47] = -122*np.cos(t2) + 123*np.sin(t1)
        cf[48] = 124*t1**4 - 125*t2**3 + 126*t1**2*t2
        cf[49] = 127*t1*t2 - 128*np.real(t1)
        cf[50] = -129*t1**3 + 130*t2**2 - 131*t1*t2
        cf[51] = 132*np.sin(t1) + 133*np.cos(t2)
        cf[52] = 134*t1**2*t2 - 135*t2**3 + 136*t1
        cf[53] = -137*t1*t2 + 138*np.imag(t2)
        cf[54] = 139*t2**2 - 140*t1**4 + 141*t1*t2
        cf[55] = 142*np.real(t1) + 143*t2**3 - 144*t1*t2**2
        cf[56] = -145*t1**3 + 146*t2 - 147*np.sin(t1*t2)
        cf[57] = 148*t1*t2 + 149*np.log(np.abs(t2) + 1)
        cf[58] = 150*t1**2 - 151*t2**2 + 152*t1*t2
        cf[59] = -153*np.cos(t2) + 154*np.sin(t1)
        cf[60] = 155*t1**4 - 156*t2**3 + 157*t1**2*t2
        cf[61] = 158*t1*t2 - 159*np.real(t1)
        cf[62] = -160*t1**3 + 161*t2**2 - 162*t1*t2
        cf[63] = 163*np.sin(t1) + 164*np.cos(t2)
        cf[64] = 165*t1**2*t2 - 166*t2**3 + 167*t1
        cf[65] = -168*t1*t2 + 169*np.imag(t2)
        cf[66] = 170*t2**2 - 171*t1**4 + 172*t1*t2
        cf[67] = 173*np.real(t1) + 174*t2**3 - 175*t1*t2**2
        cf[68] = -176*t1**3 + 177*t2 - 178*np.sin(t1*t2)
        cf[69] = 179*t1*t2 + 180*np.log(np.abs(t1) + 1)
        cf[70] = 181*t1**2 - 182*t2**2 + 183*t1*t2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_144(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + 2*t2**3 - t1*t2
        cf[1] = 3*t1**4 - t2**2 + 4*np.real(t1*t2)
        cf[2] = 5*t1**3 + 6*t2**4 - np.imag(t1**2)
        cf[3] = 7*t1**2*t2 - 8*np.abs(t2) + np.angle(t1)
        cf[4] = 9*t1*t2**2 + 10*np.cos(t1) - 11*np.sin(t2)
        cf[5] = 12*t1**3*t2 - 13*t2**3 + 14*np.log(np.abs(t1) + 1)
        cf[6] = 15*t1**2 - 16*t2 + 17*np.exp(t1*t2)
        cf[7] = 18*t1*t2**3 - 19*t1**4 + 20*(t1 + t2)
        cf[8] = 21*t1**5 - 22*t2**2 + 23*(t1 * t2)
        cf[9] = 24*t1**3*t2**2 - 25*t2**4 + 26*np.cos(t1*t2)
        cf[10] = 27*t1**2*t2**3 - 28*t1**4 + 29*np.sin(t2*t1)
        cf[11] = 30*t1*t2**4 - 31*t2**5 + 32*np.real(t1**2*t2)
        cf[12] = 33*t1**3*t2 - 34*t2**3 + 35*np.angle(t1 + t2)
        cf[13] = 36*t1**4 - 37*t2**2*t1 + 38*np.log(np.abs(t2) + 1)
        cf[14] = 39*t1*t2**3 - 40*t1**5 + 41*np.cos(t1 + t2)
        cf[15] = 42*t1**2*t2**2 - 43*t2**4 + 44*np.sin(t1 - t2)
        cf[16] = 45*t1**3*t2 - 46*t2**3*t1 + 47*np.abs(t1*t2)
        cf[17] = 48*t1**4*t2 - 49*t2**5 + 50*np.real(t1 + t2)
        cf[18] = 51*t1*t2**4 - 52*t1**5 + 53*np.angle(t1*t2)
        cf[19] = 54*t1**2*t2**3 - 55*t2**4*t1 + 56*np.log(np.abs(t1*t2) + 1)
        cf[20] = 57*t1**3*t2**2 - 58*t2**5 + 59*np.cos(t1*t2)
        cf[21] = 60*t1**4*t2 - 61*t1**5*t2 + 62*np.sin(t1 + t2)
        cf[22] = 63*t1*t2**5 - 64*t2**6 + 65*np.real(t1**2 + t2**2)
        cf[23] = 66*t1**5*t2 - 67*t2**6 + 68*np.angle(t1**2*t2)
        cf[24] = 69*t1**6 - 70*t2**3 + 71*np.log(np.abs(t1) + np.abs(t2) + 1)
        cf[25] = 72*t1**3*t2**3 - 73*t2**6 + 74*np.cos(t1**2 + t2**2)
        cf[26] = 75*t1**4*t2**2 - 76*t1**6 + 77*np.sin(t1*t2)
        cf[27] = 78*t1**5*t2 - 79*t2**7 + 80*np.real(t1*t2**2)
        cf[28] = 81*t1**6*t2 - 82*t2**7 + 83*np.angle(t1**3*t2)
        cf[29] = 84*t1**7 - 85*t2**4 + 86*np.log(np.abs(t1**2) + np.abs(t2)**2 + 1)
        cf[30] = 87*t1**4*t2**3 - 88*t2**7 + 89*np.cos(t1**3 + t2**3)
        cf[31] = 90*t1**5*t2**2 - 91*t1**7 + 92*np.sin(t1**2*t2)
        cf[32] = 93*t1**6*t2 - 94*t2**8 + 95*np.real(t1**2*t2**2)
        cf[33] = 96*t1**7*t2 - 97*t2**8 + 98*np.angle(t1**4*t2)
        cf[34] = 99*t1**8 - 100*t2**5 + 101*np.log(np.abs(t1**3) + np.abs(t2)**3 + 1)
        cf[35] = 102*t1**5*t2**3 - 103*t2**9 + 104*np.cos(t1**4 + t2**4)
        cf[36] = 105*t1**6*t2**2 - 106*t1**8 + 107*np.sin(t1**3*t2)
        cf[37] = 108*t1**7*t2 - 109*t2**9 + 110*np.real(t1**3*t2**2)
        cf[38] = 111*t1**8*t2 - 112*t2**10 + 113*np.angle(t1**5*t2)
        cf[39] = 114*t1**9 - 115*t2**6 + 116*np.log(np.abs(t1**4) + np.abs(t2)**4 + 1)
        cf[40] = 117*t1**6*t2**3 - 118*t2**10 + 119*np.cos(t1**5 + t2**5)
        cf[41] = 120*t1**7*t2**2 - 121*t1**9 + 122*np.sin(t1**4*t2)
        cf[43] = 123*t1**8*t2 - 124*t2**11 + 125*np.real(t1**4*t2**2)
        cf[44] = 126*t1**9*t2 - 127*t2**11 + 128*np.angle(t1**6*t2)
        cf[45] = 129*t1**10 - 130*t2**7 + 131*np.log(np.abs(t1**5) + np.abs(t2)**5 + 1)
        cf[46] = 132*t1**7*t2**3 - 133*t2**12 + 134*np.cos(t1**6 + t2**6)
        cf[47] = 135*t1**8*t2**2 - 136*t1**10 + 137*np.sin(t1**5*t2)
        cf[48] = 138*t1**9*t2 - 139*t2**12 + 140*np.real(t1**5*t2**2)
        cf[49] = 141*t1**10*t2 - 142*t2**13 + 143*np.angle(t1**7*t2)
        cf[50] = 144*t1**11 - 145*t2**8 + 146*np.log(np.abs(t1**6) + np.abs(t2)**6 + 1)
        cf[51] = 147*t1**8*t2**3 - 148*t2**13 + 149*np.cos(t1**7 + t2**7)
        cf[52] = 150*t1**9*t2**2 - 151*t1**11 + 152*np.sin(t1**6*t2)
        cf[53] = 153*t1**10*t2 - 154*t2**14 + 155*np.real(t1**6*t2**2)
        cf[54] = 156*t1**11*t2 - 157*t2**14 + 158*np.angle(t1**8*t2)
        cf[55] = 159*t1**12 - 160*t2**9 + 161*np.log(np.abs(t1**7) + np.abs(t2)**7 + 1)
        cf[56] = 162*t1**9*t2**3 - 163*t2**15 + 164*np.cos(t1**8 + t2**8)
        cf[57] = 165*t1**10*t2**2 - 166*t1**12 + 167*np.sin(t1**7*t2)
        cf[58] = 168*t1**11*t2 - 169*t2**15 + 170*np.real(t1**7*t2**2)
        cf[59] = 171*t1**12*t2 - 172*t2**16 + 173*np.angle(t1**9*t2)
        cf[60] = 174*t1**13 - 175*t2**10 + 176*np.log(np.abs(t1**8) + np.abs(t2)**8 + 1)
        cf[61] = 177*t1**10*t2**3 - 178*t2**16 + 179*np.cos(t1**9 + t2**9)
        cf[62] = 180*t1**11*t2**2 - 181*t1**13 + 182*np.sin(t1**8*t2)
        cf[63] = 183*t1**12*t2 - 184*t2**17 + 185*np.real(t1**8*t2**2)
        cf[64] = 186*t1**13*t2 - 187*t2**17 + 188*np.angle(t1**10*t2)
        cf[65] = 189*t1**14 - 190*t2**11 + 191*np.log(np.abs(t1**9) + np.abs(t2)**9 + 1)
        cf[66] = 192*t1**11*t2**3 - 193*t2**18 + 194*np.cos(t1**10 + t2**10)
        cf[67] = 195*t1**12*t2**2 - 196*t1**14 + 197*np.sin(t1**9*t2)
        cf[68] = 198*t1**13*t2 - 199*t2**18 + 200*np.real(t1**9*t2**2)
        cf[69] = 201*t1**14*t2 - 202*t2**19 + 203*np.angle(t1**11*t2)
        cf[70] = 204*t1**15 - 205*t2**12 + 206*np.log(np.abs(t1**10) + np.abs(t2)**10 + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_145(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k-1] = ((t1**k + np.conj(t2)**k) * (-1)**k) / (k + np.real(t1) + np.real(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_146(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0:10] = (t1**np.arange(1, 11) + t2**np.arange(1, 11)) * (-1)**np.arange(1, 11)
        cf[10:20] = np.sin(t1 * np.arange(11, 21)) - np.cos(t2 * np.arange(11, 21))
        cf[20:30] = np.log(np.abs(t1) + 1) * np.arange(21, 31) - np.log(np.abs(t2) + 1)
        cf[30:40] = (t1 * t2)**np.arange(31, 41) / (1 + np.arange(31, 41))
        cf[40:71] = np.real(t1) * np.imag(t2) - np.imag(t1) * np.real(t2) + np.angle(t1 + t2) * np.abs(t1 - t2) * np.arange(40, 71)
        cf[70] = np.sum(cf[0:70])
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_147(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + t2**4
        cf[1] = t1**4 - t2**3
        cf[2] = t1**3 * t2 + t1**2
        cf[3] = t1**2 - t2**5
        cf[4] = t2**4 - t1**3
        cf[5] = np.real(t1) * np.imag(t2) + np.sin(t1 * t2)
        cf[6] = np.cos(t2) + t1 * t2**2
        cf[7] = np.log(np.abs(t1) + 1) - np.log(np.abs(t2) + 1)
        cf[8] = t1**6 - t2**6
        cf[9] = t1 * t2**3 - t2 * t1**4
        for k in range(11, 71):
            cf[k-1] = (t1**(71 - k) + t2**(71 - k)) * (-1)**k / (k**2)
        cf[70] = np.real(t1)**2 + np.imag(t2)**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_148(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**4 - 3*t2 + np.real(t1*t2)
        cf[1] = np.imag(t1)**2 + np.abs(t2)**3
        cf[2] = np.sin(t1) + np.cos(t2)
        cf[3] = np.angle(t1) * np.angle(t2)
        cf[4] = np.log(np.abs(t1 + t2) + 1)
        for j in range(6, 71):
            cf[j-1] = (t1**j + t2**(71 - j)) / (j - 5) + (-1)**j * np.abs(t1 - t2)
        cf[70] = t1 * t2 / (1 + np.abs(t1)**2 + np.abs(t2)**2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_149(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**4 + 2*t2**3
        cf[1] = -t1**3 + 3*t2**2
        cf[2] = t1**2 - 4*t2
        cf[3] = -t1 + 5*t2**2
        cf[4] = t2**3 - 6*t1**2*t2
        for j in range(6, 36):
            cf[j-1] = (np.real(t1)**j - np.imag(t2)**j) * (-1)**j / j
        for j in range(36, 71):
            cf[j-1] = (np.sin(j * t1) + np.cos(j * t2)) / (j + 1)
        cf[70] = np.sum(np.real(t1), np.imag(t2)) * (t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_150(t1, t2):
    try:
        j = np.arange(0, 71).astype(complex)
        cf = (t1 + j * t2) * (-1)**j * np.log(np.abs(t1) + np.abs(t2) + 1)**(np.abs(j) % 5 + 1) * (j + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_151(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = (t1 + t2)**8 + (t1 - t2)**7
        cf[1] = np.sin(t1) * np.cos(t2)**6
        cf[2] = np.log(np.abs(t1 * t2) + 1) * (t1**2 - t2**2)
        cf[3] = np.real(t1) * np.imag(t2) * (t1 + t2)
        cf[4] = np.abs(t1 - t2) * np.angle(t1 + np.conj(t2))
        cf[5] = (t1**3 + t2**3) / (1 + np.abs(t1 + t2))
        cf[6] = (t1 - t2)**4 * (np.sin(t1) - np.cos(t2))
        cf[7] = (np.real(t1)**2 + np.imag(t2)**2) * np.log(np.abs(t1 - t2) + 1)
        cf[8] = (t1 * t2)**5 - (t1 + t2)**5
        cf[9] = np.sin(t1 * t2) + np.cos(t1 + t2)
        for j in range(11, 71):
            cf[j-1] = ((t1**j + t2**j) / (j + 1)) * (-1)**j
        cf[70] = 1
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_152(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        k = np.arange(1, 72)
        cf = (t1**k + np.conj(t2)**k) * (-1)**k / (1 + k)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_153(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            if k <= 35:
                cf[k-1] = ((np.real(t1)**k + np.imag(t2)**k) * (-1)**k) / (k)
            else:
                cf[k-1] = (np.sin(t1 * k) + np.cos(t2 * k)) * (-1)**k / (71 - k + 1)
        cf[70] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_154(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.real(t1)**10 + np.real(t2)**10
        cf[1] = np.imag(t1)**9 - np.imag(t2)**9
        for j in range(3, 36):
            cf[j-1] = (-1)**j * (np.real(t1)**j + np.real(t2)**(j-1)) / j
        for j in range(36, 71):
            cf[j-1] = (np.log(np.abs(t1) + 1) * j) - (np.log(np.abs(t2) + 1) / j)
        cf[70] = np.real(t1 * t2) + np.imag(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_155(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = (t1 + t2)**5 + np.real(t1) * np.imag(t2)
        cf[1] = (t1**2 - t2**2) * np.sin(t1 * t2)
        cf[2] = np.log(np.abs(t1) + 1) + np.cos(t2)
        cf[3] = np.real(t1)**2 - np.imag(t2)**2 + t1 * t2
        cf[4] = (t1 - t2)**3 + np.real(t2) * np.imag(t1)
        cf[5] = np.sin(t1 + t2) * np.cos(t1 - t2)
        cf[6] = np.real(t1 * t2) + np.imag(t1 + t2)
        cf[7] = (t1**3 + t2**3) / (np.abs(t1) + np.abs(t2) + 1)
        cf[8] = np.real(t1)**3 - np.imag(t2)**3 + t1**2 * t2
        cf[9] = np.cos(t1 * t2) + np.sin(t1 + t2)
        cf[10] = (t1 + t2)**4 - np.real(t1**2 - t2**2)
        cf[11] = np.real(t1)**2 * np.imag(t2) + np.real(t2)**2 * np.imag(t1)
        cf[12] = np.log(np.abs(t1) + np.abs(t2) + 1) * np.sin(t1 - t2)
        cf[13] = np.real(t1**2 + t2**2) - np.imag(t1 * t2)
        cf[14] = (t1 - t2)**4 + np.cos(t1**2 + t2**2)
        cf[15] = np.real(t1)**4 - np.imag(t2)**4 + t1**3 * t2
        cf[16] = np.sin(t1**2 - t2**2) * np.cos(t1 + t2)
        cf[17] = (np.real(t1) + np.imag(t2))**2 + (np.real(t2) - np.imag(t1))**2
        cf[18] = np.log(np.abs(t1 * t2) + 1) * (t1 + t2)
        cf[19] = np.real(t1**3 + t2**3) - np.imag(t1**3 - t2**3)
        cf[20] = np.cos(t1**3 - t2**3) + np.sin(t1**2 + t2**2)
        cf[21] = (t1 + t2)**5 - np.real(t1**4 - t2**4)
        cf[22] = np.real(t1**4) * np.imag(t2**2) + np.real(t2**4) * np.imag(t1**2)
        cf[23] = np.log(np.abs(t1**2) + np.abs(t2**2) + 1) * np.sin(t1**2 - t2**2)
        cf[24] = np.real(t1**4 + t2**4) - np.imag(t1**4 - t2**4)
        cf[25] = (t1 - t2)**5 + np.cos(t1**4 + t2**4)
        cf[26] = np.real(t1**5) - np.imag(t2**5) + t1**4 * t2
        cf[27] = np.sin(t1**3 - t2**3) * np.cos(t1**2 + t2**2)
        cf[28] = (np.real(t1**2) + np.imag(t2**2))**2 + (np.real(t2**2) - np.imag(t1**2))**2
        cf[29] = np.log(np.abs(t1**2 * t2**2) + 1) * (t1**2 + t2**2)
        cf[30] = np.real(t1**5 + t2**5) - np.imag(t1**5 - t2**5)
        cf[31] = np.cos(t1**5 - t2**5) + np.sin(t1**4 + t2**4)
        cf[32] = (t1 + t2)**6 - np.real(t1**6 - t2**6)
        cf[33] = np.real(t1**6) * np.imag(t2**3) + np.real(t2**6) * np.imag(t1**3)
        cf[34] = np.log(np.abs(t1**3) + np.abs(t2**3) + 1) * np.sin(t1**3 - t2**3)
        cf[35] = np.real(t1**6 + t2**6) - np.imag(t1**6 - t2**6)
        cf[36] = (t1 - t2)**6 + np.cos(t1**6 + t2**6)
        cf[37] = np.real(t1**7) - np.imag(t2**7) + t1**6 * t2
        cf[38] = np.sin(t1**4 - t2**4) * np.cos(t1**3 + t2**3)
        cf[39] = (np.real(t1**3) + np.imag(t2**3))**2 + (np.real(t2**3) - np.imag(t1**3))**2
        cf[40] = np.log(np.abs(t1**3 * t2**3) + 1) * (t1**3 + t2**3)
        cf[41] = np.real(t1**7 + t2**7) - np.imag(t1**7 - t2**7)
        cf[42] = np.cos(t1**7 - t2**7) + np.sin(t1**6 + t2**6)
        cf[43] = (t1 + t2)**7 - np.real(t1**8 - t2**8)
        cf[44] = np.real(t1**8) * np.imag(t2**4) + np.real(t2**8) * np.imag(t1**4)
        cf[45] = np.log(np.abs(t1**4) + np.abs(t2**4) + 1) * np.sin(t1**4 - t2**4)
        cf[46] = np.real(t1**8 + t2**8) - np.imag(t1**8 - t2**8)
        cf[47] = (t1 - t2)**7 + np.cos(t1**8 + t2**8)
        cf[48] = np.real(t1**9) - np.imag(t2**9) + t1**8 * t2
        cf[49] = np.sin(t1**5 - t2**5) * np.cos(t1**4 + t2**4)
        cf[50] = (np.real(t1**4) + np.imag(t2**4))**2 + (np.real(t2**4) - np.imag(t1**4))**2
        cf[51] = np.log(np.abs(t1**4 * t2**4) + 1) * (t1**4 + t2**4)
        cf[52] = np.real(t1**9 + t2**9) - np.imag(t1**9 - t2**9)
        cf[53] = np.cos(t1**9 - t2**9) + np.sin(t1**8 + t2**8)
        cf[54] = (t1 + t2)**8 - np.real(t1**10 - t2**10)
        cf[55] = np.real(t1**10) * np.imag(t2**5) + np.real(t2**10) * np.imag(t1**5)
        cf[56] = np.log(np.abs(t1**5) + np.abs(t2**5) + 1) * np.sin(t1**5 - t2**5)
        cf[57] = np.real(t1**10 + t2**10) - np.imag(t1**10 - t2**10)
        cf[58] = (t1 - t2)**8 + np.cos(t1**10 + t2**10)
        cf[59] = np.real(t1**11) - np.imag(t2**11) + t1**10 * t2
        cf[60] = np.sin(t1**6 - t2**6) * np.cos(t1**5 + t2**5)
        cf[61] = (np.real(t1**5) + np.imag(t2**5))**2 + (np.real(t2**5) - np.imag(t1**5))**2
        cf[62] = np.log(np.abs(t1**5 * t2**5) + 1) * (t1**5 + t2**5)
        cf[63] = np.real(t1**11 + t2**11) - np.imag(t1**11 - t2**11)
        cf[64] = np.cos(t1**11 - t2**11) + np.sin(t1**10 + t2**10)
        cf[65] = (t1 + t2)**9 - np.real(t1**12 - t2**12)
        cf[66] = np.real(t1**12) * np.imag(t2**6) + np.real(t2**12) * np.imag(t1**6)
        cf[67] = np.log(np.abs(t1**6) + np.abs(t2**6) + 1) * np.sin(t1**6 - t2**6)
        cf[68] = np.real(t1**12 + t2**12) - np.imag(t1**12 - t2**12)
        cf[69] = (t1 - t2)**9 + np.cos(t1**12 + t2**12)
        cf[70] = np.real(t1**13) - np.imag(t2**13) + t1**12 * t2
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_156(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + 2 * t2
        cf[1] = np.real(t1) * t2 - np.imag(t2)
        cf[2] = np.sin(t1) + np.cos(t2)
        cf[3] = np.log(np.abs(t1) + 1) + np.angle(t2)
        cf[4] = t1**2 - t2**3
        cf[5] = np.real(t1)**2 + np.imag(t2)**2
        cf[6] = np.sin(t1 * t2) - np.cos(t1)
        cf[7] = np.angle(t1)**3 + np.abs(t2)
        cf[8] = t1 * t2 + np.real(t1 + t2)
        cf[9] = np.real(t1)**3 - np.imag(t2)**3
        cf[10] = np.log(np.abs(t1) * np.abs(t2) + 1)
        cf[11] = t1**4 + t2**4
        cf[12] = np.sin(t1) * np.cos(t2)
        cf[13] = np.real(t1 * t2) - np.imag(t1 / t2)
        cf[14] = np.angle(t1 + t2)**2
        cf[15] = np.abs(t1 + t2)**3
        cf[16] = t1**5 - t2**2
        cf[17] = np.sin(np.real(t1)) + np.cos(np.imag(t2))
        cf[18] = np.log(np.abs(t1) + np.abs(t2))
        cf[19] = np.real(t1**2) * np.imag(t2**2)
        cf[20] = t1 * t2**3 - t1**3 * t2
        cf[21] = np.sin(t1 + t2) + np.cos(t1 - t2)
        cf[22] = np.real(t1)**4 - np.imag(t2)**4
        cf[23] = np.angle(t1)**2 + np.angle(t2)**2
        cf[24] = np.abs(t1)**5 - np.abs(t2)**5
        cf[25] = np.sin(t1**2) + np.cos(t2**2)
        cf[26] = np.log(np.abs(t1 * t2) + 1)
        cf[27] = np.real(t1 + t2)**3
        cf[28] = np.imag(t1 * t2)**2
        cf[29] = t1**6 + t2**6
        cf[30] = np.sin(t1 * t2) - np.cos(t1**2)
        cf[31] = np.real(t1**3) * np.imag(t2**3)
        cf[32] = np.log(np.abs(t1)**2 + np.abs(t2)**2)
        cf[33] = t1**7 - t2**4
        cf[34] = np.sin(np.real(t1 * t2)) + np.cos(np.imag(t1) + np.imag(t2))
        cf[35] = np.real(t1**4) - np.imag(t2**5)
        cf[36] = np.angle(t1 + t2)**3
        cf[37] = np.abs(t1 + t2)**4
        cf[38] = t1**8 - t2**3
        cf[39] = np.sin(t1**3) + np.cos(t2**3)
        cf[40] = np.log(np.abs(t1 * t2**2) + 1)
        cf[41] = np.real(t1**5) * np.imag(t2**4)
        cf[42] = t1**9 + t2**5
        cf[43] = np.sin(t1 * t2**2) - np.cos(t1**4)
        cf[44] = np.real(t1**6) - np.imag(t2**6)
        cf[45] = np.angle(t1 + t2)**4
        cf[46] = np.abs(t1 + t2)**5
        cf[47] = t1**10 - t2**0.5
        cf[48] = np.sin(t1**5) + np.cos(t2**5)
        cf[49] = np.log(np.abs(t1**2 * t2) + 1)
        cf[50] = np.real(t1**7) * np.imag(t2**5)
        cf[51] = t1**11 + t2**6
        cf[52] = np.sin(t1 * t2**3) - np.cos(t1**5)
        cf[53] = np.real(t1**8) - np.imag(t2**7)
        cf[54] = np.angle(t1 + t2)**5
        cf[55] = np.abs(t1 + t2)**6
        cf[56] = t1**12 - t2**0.5
        cf[57] = np.sin(t1**5) + np.cos(t2**5)
        cf[58] = np.log(np.abs(t1**3 * t2) + 1)
        cf[59] = np.real(t1**9) * np.imag(t2**6)
        cf[60] = t1**13 + t2**7
        cf[61] = np.sin(t1 * t2**4) - np.cos(t1**6)
        cf[62] = np.real(t1**10) - np.imag(t2**8)
        cf[63] = np.angle(t1 + t2)**6
        cf[64] = np.abs(t1 + t2)**7
        cf[65] = t1**14 - t2**0.5
        cf[66] = np.sin(t1**6) + np.cos(t2**6)
        cf[67] = np.log(np.abs(t1**4 * t2) + 1)
        cf[68] = np.real(t1**11) * np.imag(t2**7)
        cf[69] = t1**15 + t2**8
        cf[70] = np.sin(t1 * t2**5) - np.cos(t1**7)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_157(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        k = np.arange(71)
        cf = ((np.real(t1) + np.imag(t2))**(70 - k) + (np.real(t1) - np.imag(t2))**k) * (-1)**k / (k + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_158(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k-1] = (np.real(t1) + np.imag(t2) * k)**(1 + k / 10) * (-1)**k + np.sin(k * np.angle(t1 * t2)) + np.cos(k * np.abs(t1 + t2))
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_159(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + 2 * t2**4
        cf[1] = t1**4 - t2**3 + np.real(t1 * t2)
        cf[2] = np.sin(t1) + np.cos(t2) + t1 * t2
        cf[3] = np.log(np.abs(t1) + 1) - np.log(np.abs(t2) + 1)
        cf[4] = t1**3 - 2 * t2**2 + np.real(t1)**2
        for j in range(6, 36):
            cf[j-1] = (t1**(j-1) + (-1)**j * t2**(j-2)) / j
        for j in range(36, 71):
            cf[j-1] = np.real(t1)**j + np.imag(t2)**(j/2) * (-1)**j
        cf[70] = (t1 + t2)**2 / (1 + t1 * t2)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_160(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**3 - 2 * t2 + np.sin(t1)
        cf[1] = np.conj(t1) * t2 - np.cos(t2)
        cf[2] = t1**2 + t2**2 + np.real(t1 * t2)
        cf[3] = t1 - t2 + np.log(np.abs(t1) + 1)
        cf[4] = t1 * t2 - np.abs(t1) + np.sin(t2)
        cf[5] = np.real(t1)**2 - np.imag(t2)**2 + np.cos(t1)
        cf[6] = t1**4 - t2**3 + np.real(t1 + t2)
        cf[7] = np.imag(t1 * t2) + np.log(np.abs(t2) + 1)
        cf[8] = t1**5 - t1 * t2 + np.sin(t1 * t2)
        cf[9] = np.conj(t1)**2 + t2**4
        cf[10] = t1 * t2**2 - np.real(t2)
        cf[11] = t1**6 + np.imag(t1) - np.cos(t2)
        cf[12] = t1**3 * t2 - np.log(np.abs(t1) - np.abs(t2) + 1)
        cf[13] = t1 + t2**5 + np.sin(t1 + t2)
        cf[14] = np.conj(t1)**3 - t2**2
        cf[15] = np.real(t1 * t2**3) + np.cos(t1 * t2)
        cf[16] = t1**2 - t2**4 + np.sin(t1**2)
        cf[17] = t1 * t2**4 - np.log(np.abs(t1 * t2) + 1)
        cf[18] = t1**7 + np.real(t2**3)
        cf[19] = np.imag(t1**2 * t2) - np.cos(t2**2)
        cf[20] = t1**3 - t2**5 + np.sin(t1 - t2)
        cf[21] = np.conj(t1) * t2**2 + np.log(np.abs(t2) + 1)
        cf[22] = t1**4 + t2**6 - np.real(t1 * t2)
        cf[23] = t1 * t2**5 + np.imag(t1 + t2)
        cf[24] = t1**5 - t2**7 + np.sin(t1 * t2)
        cf[25] = np.real(t1**3 * t2**2) - np.cos(t1)
        cf[26] = t1**6 + t2**8 + np.log(np.abs(t1) + np.abs(t2))
        cf[27] = t1 * t2**6 - np.real(t2**3)
        cf[28] = t1**7 - t2**9 + np.sin(t1**2)
        cf[29] = np.conj(t1**2) * t2**3 + np.cos(t2)
        cf[30] = t1**8 + t2**10 - np.real(t1 * t2**2)
        cf[31] = t1 * t2**7 + np.imag(t1**2)
        cf[32] = t1**9 - t2**11 + np.log(np.abs(t1**2) + 1)
        cf[33] = np.real(t1**4 * t2**3) - np.sin(t2)
        cf[34] = t1**10 + t2**12 + np.cos(t1 * t2)
        cf[35] = t1 * t2**8 - np.real(t2**4)
        cf[36] = t1**11 - t2**13 + np.sin(t1**3)
        cf[37] = np.conj(t1**3) * t2**4 + np.log(np.abs(t2**2) + 1)
        cf[38] = t1**12 + t2**14 - np.real(t1 * t2**3)
        cf[39] = t1 * t2**9 + np.imag(t1**3)
        cf[40] = t1**13 - t2**15 + np.sin(t1 * t2**2)
        cf[41] = np.real(t1**5 * t2**4) - np.cos(t2**2)
        cf[42] = t1**14 + t2**16 + np.log(np.abs(t1**3) + 1)
        cf[43] = t1 * t2**10 - np.real(t2**5)
        cf[44] = t1**15 - t2**17 + np.sin(t1**4)
        cf[45] = np.conj(t1**4) * t2**5 + np.cos(t2**3)
        cf[46] = t1**16 + t2**18 - np.real(t1 * t2**4)
        cf[47] = t1 * t2**11 + np.imag(t1**4)
        cf[48] = t1**17 - t2**19 + np.log(np.abs(t1**4) + 1)
        cf[49] = np.real(t1**6 * t2**5) - np.sin(t2**3)
        cf[50] = t1**18 + t2**20 + np.cos(t1 * t2**2)
        cf[51] = t1 * t2**12 - np.real(t2**6)
        cf[52] = t1**19 - t2**21 + np.sin(t1**5)
        cf[53] = np.conj(t1**5) * t2**6 + np.log(np.abs(t2**3) + 1)
        cf[54] = t1**20 + t2**22 - np.real(t1 * t2**5)
        cf[55] = t1 * t2**13 + np.imag(t1**5)
        cf[56] = t1**21 - t2**23 + np.sin(t1 * t2**3)
        cf[57] = np.real(t1**7 * t2**6) - np.cos(t2**4)
        cf[58] = t1**22 + t2**24 + np.log(np.abs(t1**5) + 1)
        cf[59] = t1 * t2**14 - np.real(t2**7)
        cf[60] = t1**23 - t2**25 + np.sin(t1**6)
        cf[61] = np.conj(t1**6) * t2**7 + np.cos(t2**5)
        cf[62] = t1**24 + t2**26 - np.real(t1 * t2**6)
        cf[63] = t1 * t2**15 + np.imag(t1**6)
        cf[64] = t1**25 - t2**27 + np.log(np.abs(t1**6) + 1)
        cf[65] = np.real(t1**8 * t2**7) - np.sin(t2**5)
        cf[66] = t1**26 + t2**28 + np.cos(t1 * t2**3)
        cf[67] = t1 * t2**16 - np.real(t2**8)
        cf[68] = t1**27 - t2**29 + np.sin(t1**7)
        cf[69] = np.conj(t1**7) * t2**8 + np.log(np.abs(t2**4) + 1)
        cf[70] = t1**28 + t2**30 - np.real(t1 * t2**7)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_161(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        k = np.arange(1, 71)
        cf[:70] = (t1**((k % 4) + 1) * t2**((k % 3) + 1)) + (-1)**k * np.log(np.abs(t1) + 1) * np.sin(k * t2)
        cf[70] = t1 * t2 / (1 + t1**2 + t2**2)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_162(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**10 + 2 * t2**9
        cf[1] = 3 * t1**8 - 4 * t2**7
        cf[2] = 5 * t1**6 + 6 * t2**5
        cf[3] = 7 * t1**4 - 8 * t2**3
        cf[4] = 9 * t1**2 + 10 * t2
        cf[5] = 11 * np.conj(t1) - 12 * np.conj(t2)
        cf[6] = 13 * np.sin(t1) + 14 * np.cos(t2)
        cf[7] = 15 * np.log(np.abs(t1) + 1) - 16 * np.log(np.abs(t2) + 1)
        cf[8] = 17 * t1 * t2 + 18 * t1**3
        cf[9] = 19 * t2**2 - 20 * t1**4
        cf[10] = 21 * t1**5 + 22 * t2**5
        cf[11] = 23 * t1**6 - 24 * t2**6
        cf[12] = 25 * t1**7 + 26 * t2**7
        cf[13] = 27 * t1**8 - 28 * t2**8
        cf[14] = 29 * t1**9 + 30 * t2**9
        cf[15] = 31 * t1**10 - 32 * t2**10
        for j in range(17, 71):
            cf[j-1] = (33 + j) * t1**(j % 5) * t2**((j + 1) % 5)
        cf[70] = 71 * np.sin(t1 + t2) + 72 * np.cos(t1 - t2)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_163(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**4 + 2 * t2**3
        cf[1] = -t1**3 + 3 * t1 * t2**2
        cf[2] = 4 * t1**2 - 5 * t2
        cf[3] = -6 * t1 + 7 * t2**2
        cf[4] = 8 - 9 * t1 * t2
        cf[5] = (t1 + np.conj(t2))**2
        cf[6] = -10 * t1**3 + 11 * t2**4
        cf[7] = 12 * t1**2 * t2 - 13 * t2**3
        cf[8] = -14 * t1 * t2**2 + 15
        cf[9] = 16 * t1**4 - 17 * t2
        for j in range(11, 36):
            cf[j-1] = ((-1)**j * (t1 + j * t2)) / (j + 1)
        for j in range(36, 71):
            cf[j-1] = ((j % 2) * t1**2 - (j % 3) * t2**2) / (j + 2)
        cf[70] = t1**3 - t2**3 + 18
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_164(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            if k % 4 == 1:
                cf[k-1] = t1**k + t2**(k-1) * np.sin(t1)
            elif k % 4 == 2:
                cf[k-1] = (t1 + t2)**k * np.cos(t2)
            elif k % 4 == 3:
                cf[k-1] = np.log(np.abs(t1) + 1) * t2**k
            else:
                cf[k-1] = (t1 - t2)**k * np.sin(t1 * t2)
        cf[0] = t1 + 2 * t2
        cf[70] = t1**35 - t2**35 + 1j * t1 * t2
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_165(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**6 + t2**5
        cf[1] = t1**5 - t2**4
        cf[2] = t1**4 + t2**3
        cf[3] = -t1**3 + t2**2
        cf[4] = t1**2 - t2
        cf[5] = -t1 + 1
        for j in range(7, 72):
            cf[j-1] = (t1 * t2) / j * (-1)**j
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_166(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k-1] = t1**k + (-1)**k * t2**k / (k + 1) + np.sin(k * t1) + np.cos(k * t2)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_167(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(0, 71):
            j = k + 1
            cf[j-1] = (t1 + t2)**(70 - k) * (t1 - t2)**k * (-1)**k / (k + 1)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_168(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1**2 - t2**2
        cf[2] = np.sin(t1) * np.cos(t2)
        cf[3] = np.log(np.abs(t1) + 1) - np.log(np.abs(t2) + 1)
        cf[4] = t1 * t2
        for k in range(6, 72):
            cf[k-1] = ((t1**(k-5) + t2**(k-5)) * (-1)**k) / (k**0.5)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_169(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(1, 72):
            cf[j-1] = (t1**(j % 5 + 1) * np.conj(t2)**(j % 3 + 1)) * (-1)**j / (j + 1)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_170(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(0, 71):
            cf[j] = (t1**j + np.conj(t2)**j) * (-1)**j / (j + 2)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_171(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        k = np.arange(71)
        cf = (t1**k + t2**(70 - k)) * (-1)**k * (71 - k)
        return cf.astype(np.complex128).astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_172(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + 2 * t2
        cf[1] = np.conj(t1) - 3 * np.conj(t2)
        cf[2] = t1 * t2 + 4 * t1**2
        cf[3] = 5 * np.sin(t1) - 6 * np.cos(t2)
        cf[4] = 7 * t1**3 + 8 * t2**3
        cf[5] = 9 * t1 * t2**2 - 10 * t1**2 * t2
        cf[6] = 11 * np.log(np.abs(t1) + 1) + 12 * np.log(np.abs(t2) + 1)
        cf[7] = 13 * t1**4 - 14 * t2**4
        cf[8] = 15 * np.sin(t1 * t2) + 16 * np.cos(t1 + t2)
        cf[9] = 17 * t1**2 * t2 + 18 * t1 * t2**2
        cf[10] = 19 * np.sin(t1)**2 - 20 * np.cos(t2)**2
        cf[11] = 21 * t1**5 + 22 * t2**5
        cf[12] = 23 * np.conj(t1 * t2) - 24 * t1 * t2
        cf[13] = 25 * t1**3 * t2**2 + 26 * t1**2 * t2**3
        cf[14] = 27 * np.sin(t1**2) - 28 * np.cos(t2**2)
        cf[15] = 29 * t1**6 + 30 * t2**6
        cf[16] = 31 * t1 * t2**4 - 32 * t1**4 * t2
        cf[17] = 33 * np.log(np.abs(t1**2) + 1) + 34 * np.log(np.abs(t2**2) + 1)
        cf[18] = 35 * t1**7 - 36 * t2**7
        cf[19] = 37 * np.sin(t1 * t2**2) + 38 * np.cos(t1**2 + t2)
        cf[20] = 39 * t1**3 * t2**3 - 40 * t1**3 * t2**3
        cf[21] = 41 * np.sin(t1)**3 - 42 * np.cos(t2)**3
        cf[22] = 43 * t1**8 + 44 * t2**8
        cf[23] = 45 * t1 * t2**5 + 46 * t1**5 * t2
        cf[24] = 47 * np.log(np.abs(t1**3) + 1) - 48 * np.log(np.abs(t2**3) + 1)
        cf[25] = 49 * t1**9 - 50 * t2**9
        cf[26] = 51 * np.sin(t1 * t2**3) - 52 * np.cos(t1**3 + t2**2)
        cf[27] = 53 * t1**4 * t2**4 + 54 * t1**4 * t2**4
        cf[28] = 55 * np.sin(t1)**4 - 56 * np.cos(t2)**4
        cf[29] = 57 * t1**10 + 58 * t2**10
        cf[30] = 59 * t1 * t2**6 - 60 * t1**6 * t2
        cf[31] = 61 * np.log(np.abs(t1**4) + 1) + 62 * np.log(np.abs(t2**4) + 1)
        cf[32] = 63 * t1**11 - 64 * t2**11
        cf[33] = 65 * np.sin(t1 * t2**4) + 66 * np.cos(t1**4 + t2**3)
        cf[34] = 67 * t1**5 * t2**5 - 68 * t1**5 * t2**5
        cf[35] = 69 * np.sin(t1)**5 - 70 * np.cos(t2)**5
        cf[36] = 71 * t1**12 + 72 * t2**12
        cf[37] = 73 * t1 * t2**7 + 74 * t1**7 * t2
        cf[38] = 75 * np.log(np.abs(t1**5) + 1) - 76 * np.log(np.abs(t2**5) + 1)
        cf[39] = 77 * t1**13 - 78 * t2**13
        cf[40] = 79 * np.sin(t1 * t2**5) - 80 * np.cos(t1**5 + t2**4)
        cf[41] = 81 * t1**6 * t2**6 + 82 * t1**6 * t2**6
        cf[42] = 83 * np.sin(t1)**6 - 84 * np.cos(t2)**6
        cf[43] = 85 * t1**14 + 86 * t2**14
        cf[44] = 87 * t1 * t2**8 - 88 * t1**8 * t2
        cf[45] = 89 * np.log(np.abs(t1**6) + 1) + 90 * np.log(np.abs(t2**6) + 1)
        cf[46] = 91 * t1**15 - 92 * t2**15
        cf[47] = 93 * np.sin(t1 * t2**6) + 94 * np.cos(t1**6 + t2**5)
        cf[48] = 95 * t1**7 * t2**7 - 96 * t1**7 * t2**7
        cf[49] = 97 * np.sin(t1)**7 - 98 * np.cos(t2)**7
        cf[50] = 99 * t1**16 + 100 * t2**16
        cf[51] = 101 * t1 * t2**9 + 102 * t1**9 * t2
        cf[52] = 103 * np.log(np.abs(t1**7) + 1) - 104 * np.log(np.abs(t2**7) + 1)
        cf[53] = 105 * t1**17 - 106 * t2**17
        cf[54] = 107 * np.sin(t1 * t2**7) - 108 * np.cos(t1**7 + t2**6)
        cf[55] = 109 * t1**8 * t2**8 + 110 * t1**8 * t2**8
        cf[56] = 111 * np.sin(t1)**8 - 112 * np.cos(t2)**8
        cf[57] = 113 * t1**18 + 114 * t2**18
        cf[58] = 115 * t1 * t2**10 - 116 * t1**10 * t2
        cf[59] = 117 * np.log(np.abs(t1**8) + 1) + 118 * np.log(np.abs(t2**8) + 1)
        cf[60] = 119 * t1**19 - 120 * t2**19
        cf[61] = 121 * np.sin(t1 * t2**8) + 122 * np.cos(t1**8 + t2**7)
        cf[62] = 123 * t1**9 * t2**9 - 124 * t1**9 * t2**9
        cf[63] = 125 * np.sin(t1)**9 - 126 * np.cos(t2)**9
        cf[64] = 127 * t1**20 + 128 * t2**20
        cf[65] = 129 * t1 * t2**11 + 130 * t1**11 * t2
        cf[66] = 131 * np.log(np.abs(t1**9) + 1) - 132 * np.log(np.abs(t2**9) + 1)
        cf[67] = 133 * t1**21 - 134 * t2**21
        cf[68] = 135 * np.sin(t1 * t2**9) - 136 * np.cos(t1**9 + t2**8)
        cf[69] = 137 * t1**10 * t2**10 + 138 * t1**10 * t2**10
        cf[70] = 139 * np.sin(t1)**10 - 140 * np.cos(t2)**10
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_173(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k-1] = (t1**(k % 5 + 1) + np.conj(t2)**(k % 7 + 1)) * (-1)**k * np.log(np.abs(t1) + np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_174(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k-1] = (t1**k + np.conj(t2)**k) * (-1)**k / (1 + k**1.2)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_175(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k-1] = ((np.real(t1)**k + np.imag(t2)**(k % 5 + 1)) * (-1)**k) / (1 + k)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_176(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(1, 72):
            cf[j-1] = ((t1**j + np.conj(t2)**(71 - j)) * (-1)**j) / np.log(j + np.abs(t1) + np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_177(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 - t2**3 + 2 * t1 * t2
        cf[1] = -t1**4 + 3 * t2**2 - t1 * t2**2
        cf[2] = t1**3 - 4 * t2 + 2 * t1**2 * t2
        cf[3] = -t1**2 + 5 * t2**4 - 3 * t1 * t2**3
        cf[4] = t1 - 6 * t2**5 + 4 * t1**2 * t2**4
        for k in range(6, 71):
            cf[k-1] = ((t1**k) * (-1)**k + t2**(k-1)) / (k + 1)
        cf[70] = np.log(np.abs(t1) + 1) + t1 * t2**2
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_178(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(0, 71):
            cf[j] = ((-1)**j * (t1**j + t2**j)) / ((j + 1)**2)
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_179(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = (t1 + t2) * (np.real(t1) - np.imag(t2))
        for j in range(2, 71):
            if j % 2 == 0:
                cf[j-1] = (np.abs(t1)**j - np.abs(t2)**j) / (j + 1) * (-1)**j
            else:
                cf[j-1] = (np.real(t1)**j + np.imag(t2)**j) / (j + 2) * np.sin(j * np.angle(t1 + t2))
        cf70a = sum((t1**n).real * (t2**n).imag for n in range(1, 6))    
        cf70b =  np.log(np.abs(t1) + np.abs(t2) + 1)   
        cf[70] =  cf70b + cf70a
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_180(t1, t2,err=False):
    try:
        cf0 = t1 ** ((np.arange(71) % 6) + 1)
        cf1 = t2 ** ((np.arange(71) % 4) + 1)
        cf2 = (-1)**np.arange(71)
        cf3 = np.log(np.arange(71) + 1)
        cf = ((cf0 + cf1) * cf2 * cf3).astype(complex)
        return cf.astype(np.complex128)
    except Exception as e:
        if err:
            print(f"Details: {e}")
        return np.zeros(71, dtype=np.complex128)

def poly_181(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(1, 72):
            cf[k-1] = (-1)**k * (np.abs(t1)**k + np.abs(t2)**k) + (np.sin(k * np.angle(t1)) - np.cos(k * np.angle(t2))) / k
        return cf.astype(np.complex128)
    except Exception as e:
        return np.zeros(0, dtype=np.complex128)

def poly_182(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            cf[j] = (t1**(j + 1) * np.conj(t2)**(70 - j)) * (-1)**(j // 5) + np.log(np.abs(t1 + (j + 1) * t2) + 1)
        cf[70] = np.sin(t1) + np.cos(t2) + np.real(t1 * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_183(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**6 + 2 * t2**5
        cf[1] = 3 * t1**5 - t2**4
        cf[2] = 4 * t1**4 + 2 * t2**3
        cf[3] = 5 * t1**3 - 3 * t2**2
        cf[4] = 6 * t1**2 + 4 * t2
        cf[5] = 7 * t1 - 5
        for j in range(6, 36):
            cf[j] = ((-1)**j) * (t1 + 1)**j + (t2 - 1)**j
        for j in range(36, 71):
            cf[j] = (j - 35) * np.sin(t1 * j) - (j - 34) * np.cos(t2 * j)
        cf[70] = np.sum([t1**2, t2**2]) + np.prod([t1, t2])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_184(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = t1**5 + 2 * t2**4
        cf[1] = -3 * t1**4 + 4 * t2**3
        cf[2] = 5 * t1**3 - 6 * t2**2
        cf[3] = -7 * t1**2 + 8 * t2
        cf[4] = 9 * t1 - 10 * t2**0
        cf[5:10] = (t1 * t2)**np.arange(1, 6) * np.array([11, -12, 13, -14, 15])
        cf[10:20] = (t1 + t2)**(np.arange(6, 16) / 2) * np.array([-16, 17, -18, 19, -20, 21, -22, 23, -24, 25])
        cf[20:30] = (t1 - t2)**(np.arange(16, 26) / 3) * np.array([26, -27, 28, -29, 30, 31, -32, 33, -34, 35])
        cf[30:40] = (t1 * t2)**(np.arange(26, 36) / 4) * np.array([36, -37, 38, -39, 40, 41, -42, 43, -44, 45])
        cf[40:50] = (t1 + np.conj(t2))**(np.arange(36, 46) / 5) * np.array([-46, 47, -48, 49, -50, 51, -52, 53, -54, 55])
        cf[50:60] = (np.conj(t1) - t2)**(np.arange(46, 56) / 6) * np.array([56, -57, 58, -59, 60, 61, -62, 63, -64, 65])
        cf[60:70] = (np.abs(t1) + np.abs(t2))**(np.arange(56, 66) / 7) * np.array([-66, 67, -68, 69, -70, 71, -72, 73, -74, 75])
        cf[70] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_185(t1, t2):
    cf = np.zeros(71, dtype=np.complex128)
    try:
        cf[0] = t1**5 + 2 * t2**4
        cf[1] = -3 * t1 * t2 + 4 * np.log(np.abs(t1) + 1)
        cf[2] = 5 * t1**2 - 6 * t2**3
        cf[3] = 7 * np.sin(t1) - 8 * np.cos(t2)
        cf[4] = 9 * t1**3 + 10 * t2**4
        cf[5] = -11 * t1 * t2**2 + 12 * t2 * t1**2
        cf[6] = 13 * t1**2 * t2 - 14 * t2**2 * t1
        cf[7] = 15 * np.sin(t1 * t2) + 16 * np.cos(t1 + t2)
        cf[8] = -17 * t1**4 + 18 * t2**5
        cf[9] = 19 * t1 * t2**3 - 20 * t2 * t1**3
        cf[10] = 21 * t1**3 * t2**2
        cf[11] = -22 * np.sin(t1**2) + 23 * np.cos(t2**2)
        cf[12] = 24 * t1**5 - 25 * t2**6
        cf[13] = 26 * t1**2 * t2**4 - 27 * t2**2 * t1**4
        cf[14] = 28 * np.sin(t1 * t2**2) - 29 * np.cos(t1**2 * t2)
        cf[15] = 30 * t1**6 + 31 * t2**7
        cf[16] = -32 * t1**3 * t2**3 + 33 * t2**3 * t1**3
        cf[17] = 34 * t1 * t2**5 - 35 * t2 * t1**5
        cf[18] = 36 * np.sin(t1**3) + 37 * np.cos(t2**3)
        cf[19] = 38 * t1**7 - 39 * t2**8
        cf[20] = 40 * t1**2 * t2**6 - 41 * t2**2 * t1**6
        cf[21] = 42 * np.sin(t1 * t2**3) - 43 * np.cos(t1**3 * t2)
        cf[22] = 44 * t1**8 + 45 * t2**9
        cf[23] = -46 * t1**4 * t2**4 + 47 * t2**4 * t1**4
        cf[24] = 48 * t1 * t2**7 - 49 * t2 * t1**7
        cf[25] = 50 * np.sin(t1**4) + 51 * np.cos(t2**4)
        cf[26] = 52 * t1**9 - 53 * t2**10
        cf[27] = 54 * t1**2 * t2**8 - 55 * t2**2 * t1**8
        cf[28] = 56 * np.sin(t1 * t2**4) - 57 * np.cos(t1**4 * t2)
        cf[29] = 58 * t1**10 + 59 * t2**11
        cf[30] = -60 * t1**5 * t2**5 + 61 * t2**5 * t1**5
        cf[31] = 62 * t1 * t2**9 - 63 * t2 * t1**9
        cf[32] = 64 * np.sin(t1**5) + 65 * np.cos(t2**5)
        cf[33] = 66 * t1**11 - 67 * t2**12
        cf[34] = 68 * t1**2 * t2**10 - 69 * t2**2 * t1**10
        cf[35] = 70 * np.sin(t1 * t2**5) - 71 * np.cos(t1**5 * t2)
        cf[36] = 72 * t1**12 + 73 * t2**13
        cf[37] = -74 * t1**6 * t2**6 + 75 * t2**6 * t1**6
        cf[38] = 76 * t1 * t2**11 - 77 * t2 * t1**11
        cf[39] = 78 * np.sin(t1**6) + 79 * np.cos(t2**6)
        cf[40] = 80 * t1**13 - 81 * t2**14
        cf[41] = 82 * t1**2 * t2**12 - 83 * t2**2 * t1**12
        cf[42] = 84 * np.sin(t1 * t2**6) - 85 * np.cos(t1**6 * t2)
        cf[43] = 86 * t1**14 + 87 * t2**15
        cf[44] = -88 * t1**7 * t2**7 + 89 * t2**7 * t1**7
        cf[45] = 90 * t1 * t2**13 - 91 * t2 * t1**13
        cf[46] = 92 * np.sin(t1**7) + 93 * np.cos(t2**7)
        cf[47] = 94 * t1**15 - 95 * t2**16
        cf[48] = 96 * t1**2 * t2**14 - 97 * t2**2 * t1**14
        cf[49] = 98 * np.sin(t1 * t2**7) - 99 * np.cos(t1**7 * t2)
        cf[50] = 100 * t1**16 + 101 * t2**17
        cf[51] = -102 * t1**8 * t2**8 + 103 * t2**8 * t1**8
        cf[52] = 104 * t1 * t2**15 - 105 * t2 * t1**15
        cf[53] = 106 * np.sin(t1**8) + 107 * np.cos(t2**8)
        cf[54] = 108 * t1**17 - 109 * t2**18
        cf[55] = 110 * t1**2 * t2**16 - 111 * t2**2 * t1**16
        cf[56] = 112 * np.sin(t1 * t2**8) - 113 * np.cos(t1**8 * t2)
        cf[57] = 114 * t1**18 + 115 * t2**19
        cf[58] = -116 * t1**9 * t2**9 + 117 * t2**9 * t1**9
        cf[59] = 118 * t1 * t2**17 - 119 * t2 * t1**17
        cf[60] = 120 * np.sin(t1**9) + 121 * np.cos(t2**9)
        cf[61] = 122 * t1**19 - 123 * t2**20
        cf[62] = 124 * t1**2 * t2**18 - 125 * t2**2 * t1**18
        cf[63] = 126 * np.sin(t1 * t2**9) - 127 * np.cos(t1**9 * t2)
        cf[64] = 128 * t1**20 + 129 * t2**21
        cf[65] = -130 * t1**10 * t2**10 + 131 * t2**10 * t1**10
        cf[66] = 132 * t1 * t2**19 - 133 * t2 * t1**19
        cf[67] = 134 * np.sin(t1**10) + 135 * np.cos(t2**10)
        cf[68] = 136 * t1**21 - 137 * t2**22
        cf[69] = 138 * t1**2 * t2**20 - 139 * t2**2 * t1**20
        cf[70] = 140 * np.sin(t1 * t2**10) + 141 * np.cos(t1**10 * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_186(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            cf[j] = ((-1)**j) * (np.real(t1)**j + np.imag(t2)**j) / (np.log(np.abs(t1 + t2)) + j) * np.sin(j * np.angle(t1 * t2 + 1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_187(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        k = np.arange(71)
        cf = t1**k * np.sin(t2 * k) + t2**k * np.cos(t1 * k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_188(t1, t2):
    try:
        k = np.arange(71)
        cf = (np.real(t1)**k + np.imag(t2) * k) * (-1)**k + np.log(np.abs(t1 + t2 * k) + 1) + np.sin(t1 * k) * np.cos(t2 * k) + (np.angle(t1) * k - np.angle(t2)) * 1j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_189(t1, t2):
    try:
        degrees = np.arange(71)
        cf = t1**(70 - degrees) * (np.cos(degrees) + 1j * np.sin(degrees)) + t2**degrees * (np.cos(degrees) - 1j * np.sin(degrees))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_190(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        exponents = np.arange(71)
        cf = t1**exponents + (-1)**exponents * t2**(exponents + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_191(t1, t2):
    try:
        exponents = np.arange(71)
        cf = (t1**exponents) * np.sin(exponents * np.angle(t2)) + (np.conj(t2)**exponents) * np.cos(exponents * np.real(t1))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_192(t1, t2):
    try:
        degrees = np.arange(71)
        cf = (t1**degrees) * (np.conj(t2)**(degrees % 7)) * (-1)**(degrees // 6) * (1 + degrees / 70)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_193(t1, t2):
    try:
        j = np.arange(71)
        cf = (t1**j * t2**(70 - j)) * ((-1)**j + np.real(t1) * np.imag(t2) / (j + 1))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_194(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        exponents = np.arange(71)
        cf = (t1**exponents) * np.real(t2) + (np.conj(t1)**exponents) * np.imag(t2) - np.log(np.abs(t1) + 1)**exponents + np.sin(t1 * exponents) - np.cos(t2 * exponents)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_195(t1, t2):
    try:
        degrees = np.arange(71)
        cf = np.zeros(71, dtype=np.complex128)
        cf = (t1**degrees) * np.sin(t2 * degrees) + (t2**degrees) * np.cos(t1 * degrees)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_196(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        exponents1 = np.arange(1, 72)
        exponents2 = np.arange(71, 0, -1)
        terms1 = t1**exponents1
        terms2 = (-1)**np.arange(71) * t2**exponents2
        terms3 = np.sin(t1 * np.arange(71)) * np.cos(t2 * np.arange(1, 72))
        cf = terms1 + terms2 + terms3
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_197(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        j = np.arange(71)
        cf = (np.real(t1)**j * np.sin(j * np.angle(t2)) + np.real(t2)**(70 - j) * np.cos((70 - j) * np.angle(t2))) + \
              (np.imag(t1)**j * np.cos(j * np.angle(t2)) - np.imag(t2)**(j / 2) * np.sin((70 - j) * np.angle(t1)))
        cf = cf * (np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_198(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        k = np.arange(71)
        cf = (np.real(t1) + np.imag(t2))**(70 - k) * (np.abs(t1) + np.abs(t2))**k * np.sin(k * np.angle(t1) - np.angle(t2)) + \
             (np.real(t2) - np.imag(t1))**k * np.cos(k * np.angle(t2) + np.angle(t1)) + np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1) / (k + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_199(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        j = np.arange(71)
        cf = (np.real(t1)**j * np.sin(j * np.real(t2))) + (np.imag(t2)**j * np.cos(j * np.imag(t1))) / (j + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_200(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(71):
            cf[k] = (np.real(t1)**(70 - k) * np.sin(k * np.angle(t1)) + np.imag(t2)**k * np.cos(k * np.angle(t2))) / (1 + k)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_201(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        j = np.arange(71)
        cf = (np.real(t1)**j * np.sin(j * np.angle(t2)) + np.real(t2)**j * np.cos(j * np.angle(t1))) + \
              (np.imag(t1)**j * np.cos(j * np.angle(t2)) - np.imag(t2)**j * np.sin(j * np.angle(t1))) * 1j
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_202(t1, t2):
    try:
        k = np.arange(71)
        cf = (t1**k * np.sin(k * np.angle(t1) + np.real(t2)) + t2**k * np.cos(k * np.angle(t2) - np.real(t1))) * np.log(np.abs(t1 * t2) + 1) / (1 + k**2) + \
             (np.sin(k * np.real(t1)) - np.cos(k * np.imag(t2))) * (np.abs(t1) + np.abs(t2)) / (2 + k)
        return np.array(cf, dtype=np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_203(t1, t2):
    try:
        k = np.arange(1, 72)
        real_part = np.real(t1)**k * np.sin(k * np.angle(t2)) + np.real(t2)**k * np.cos(k * np.angle(t1)) + np.log(np.abs(t1) + k) + np.real(t1 + t2)**k / (k + 1)
        imag_part = np.imag(t1)**k * np.cos(k * np.angle(t2)) + np.imag(t2)**k * np.sin(k * np.angle(t1)) + np.sin(k) + np.cos(k)
        cf = real_part + 1j * imag_part
        cf = cf * ((-1)**k * np.log(k + np.abs(t1)))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_204(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        j = np.arange(1, 72)
        cf = np.real(t1)**j + np.real(t2)**(71 - j) * np.cos(j * np.angle(t1) + (71 - j) * np.angle(t2)) + np.log(np.abs(t1) + 1) * np.sin(j * np.angle(t2))
        cf += np.imag(t1)**j - np.imag(t2)**(71 - j) * np.sin(j * np.angle(t1) - (71 - j) * np.angle(t2)) + np.log(np.abs(t2) + 1) * np.cos(j * np.angle(t1))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_205(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            cf[j] = (np.real(t1) * (j + 1)**2 + np.imag(t2) * (j + 1)) * np.sin((j + 1) * np.angle(t1 + t2)) + \
                     (np.cos((j + 1) * np.angle(t1)) + np.log(np.abs(t1 * t2) + 1)) * (np.cos(j + 1) + np.sin(j + 1) * (0 + 1j))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_206(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            real_part = np.real(t1)**(j + 1) * np.sin((j + 1) * np.angle(t2) + np.log(np.abs(t1) + 1))
            imag_part = np.imag(t2)**(j + 1) * np.cos((j + 1) * np.angle(t1) + np.log(np.abs(t2) + 1))
            cf[j] = real_part + np.imag(t1) * np.real(t2) * imag_part
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_207(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            k = (j + 1)**2 * np.real(t1) - np.imag(t2)
            r = np.sin((j + 1) * np.angle(t1)) + np.cos((j + 1) * np.angle(t2))
            magnitude = np.log(np.abs(t1) + 1) * (k + r**2)
            angle = np.abs(t2) * (j + 1) + np.real(t1) * np.sin(j + 1)
            cf[j] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        for j in range(71):
            cf[j] += np.conj(t1) * np.conj(t2) * (j + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_208(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            mag = np.log(np.abs(t1) * (j + 1) + np.abs(t2) * (j + 1)**2 + 1)
            angle = np.angle(t1) * np.sqrt(j + 1) + np.angle(t2) * np.log(j + 2)
            cf[j] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_209(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            mag = 0
            ang = 0
            for k in range(1, min(j + 1, 11)):
                mag += np.real(t1)**k * np.real(t2)**(j - k) * np.log(np.abs(t1) + np.abs(t2) + 1)
                ang += np.angle(t1) * k - np.angle(t2) * (j - k) + np.sin(k) * np.angle(np.conj(t1 + t2))
            cf[j] = mag * (np.cos(ang) + np.sin(ang) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_210(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            mag = np.log(np.abs(t1)**(j + 1) + np.abs(t2)**(70 - j) + 1) * (1 + np.sin((j + 1) * np.real(t1)) - np.cos((j + 1) * np.imag(t2)))
            angle = np.angle(t1) * (j + 1) + np.angle(t2) * ((j + 1)**2) + np.sin((j + 1) * np.real(t1)) - np.cos((j + 1) * np.imag(t2))
            cf[j] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_211(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            mag = np.log(np.abs(t1) * (j + 1) + np.abs(t2) * np.sqrt(j + 1) + 1) + np.sin((j + 1) * np.real(t1)) * np.cos((j + 1) * np.imag(t2))
            angle = np.angle(t1)**2 / (j + 2) + np.angle(t2) * np.cos(j + 1) + np.real(t1 * t2)
            cf[j] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_212(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            cf[j] = (np.real(t1)**j + np.imag(t2)**(70 - j)) * np.cos(j * np.angle(t1 + t2)) + np.sin(j * np.angle(t1 * t2)) + np.log(np.abs(t1) + np.abs(t2) + 1)**j
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_213(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            k = (j + 1) * 3 + (j // 7)
            r = (j % 5) + 2
            mag = np.abs(t1)**k + np.abs(t2)**r + np.sum([np.real(t1), np.imag(t2)]) * np.log(np.abs(t1) + 1)
            angle = np.angle(t1) * k - np.angle(t2) * r + np.sin(j + 1) * np.cos(j + 1)
            cf[j] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_214(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        cf[0] = np.real(t1) + np.imag(t2) * 1j
        prev = t1 * t2
        for j in range(70):
            magnitude = np.log(np.abs(prev) + 1) + np.real(prev)**2 - np.imag(prev)**2
            angle = np.angle(prev) + np.sin(np.real(prev)) - np.cos(np.imag(prev))
            cf[j + 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
            prev = prev * t1 - t2 / (j + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_215(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            cf[j] = (np.real(t1) + (j + 1)) * np.sin((j + 1) * np.angle(t1)) + np.conj(t2) * (np.imag(t1) + (j + 1)) * np.cos((j + 1) * np.angle(t2)) * 1j
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_216(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            k = ((j + 1) * 5) % 20 + 1
            r = (j // 6) + 1
            cf[j] = (np.real(t1)**k + np.imag(t2)**r) * np.cos((j + 1) * np.angle(t1)) + np.conj(t2) * np.sin((j + 1) * np.angle(t2)) - np.real(t1 * t2) * np.cos(j + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_217(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            angle = np.angle(t1) * (j + 1) + np.angle(t2) * (71 - (j + 1))
            magnitude = np.abs(t1)**(j + 1) * np.abs(t2)**(71 - (j + 1)) + np.log(np.abs(t1) + 1) * np.sin(j + 1) + np.cos(j + 1)
            cf[j] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_218(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(71):
            mag = np.sin((k + 1) * np.abs(t1)) + np.cos((k + 1) * np.abs(t2)) + np.log(np.abs(t1) + (k + 1))
            angle = np.angle(t1) * (k + 1) + np.angle(t2) * (71 - (k + 1))
            cf[k] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_219(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for k in range(71):
            angle = np.angle(t1) * (k + 1) - np.angle(t2) * (71 - (k + 1))
            magnitude = np.abs(t1)**(k + 1) + np.log(np.abs(t2) + 1)**(k + 1)
            cf[k] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_220(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            cf[j] = (np.real(t1) * (j + 1) + np.imag(t2) * (j + 1)**2) * np.sin(np.angle(t1) * (j + 1)) + \
                     np.cos(np.angle(t2) * (j + 1)) * np.log(np.abs(t1) + np.abs(t2) * (j + 1)) + np.real(np.conj(t1) * t2)**(j + 1) - np.imag(t1 * np.conj(t2))**(j + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_221(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            cf[j] = (np.real(t1)**(j + 1) * np.cos(np.angle(t2) + (j + 1))) + (np.imag(t2)**(j + 1) * np.sin(np.angle(t1) * (j + 1))) + np.log(np.abs(t1) + (j + 1)) + np.log(np.abs(t2) + 1) + np.conj(t1) * (j + 1) - np.conj(t2)**(j + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_222(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        prev = t1 + t2
        for j in range(71):
            magnitude = np.abs(prev) * np.log(np.abs(prev) + 1)
            angle = np.angle(prev) + np.sin(j + 1) * np.cos(j + 1)
            cf[j] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
            prev = prev * t1 - t2 / (j + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_223(t1, t2):
    try:
        cf = np.zeros(71, dtype=np.complex128)
        for j in range(71):
            mag = np.log(np.abs(t1) * (j + 1) + 1) * (1 + np.sin((j + 1) * np.angle(t2)))
            ang = np.angle(t1) * np.sqrt(j + 1) + np.cos((j + 1) * np.angle(t2))
            cf[j] = mag * np.cos(ang) + mag * np.sin(ang) * 1j
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_224(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(35):
            if (j + 1) % 4 == 1:
                k = j + 3
                cf[j] = (np.real(t1)**k) + (np.imag(t2)**k) * np.sin(np.angle(t1) * k)
            elif (j + 1) % 4 == 2:
                k = j + 4
                cf[j] = (np.abs(t1 + t2)**k) * np.cos(np.angle(t2) * k) + np.log(np.abs(t1) + 1)
            elif (j + 1) % 4 == 3:
                k = j + 2
                cf[j] = np.conj(t1)**k - np.conj(t2)**k + np.sin(t1 * k) - np.cos(t2 * k)
            else:
                k = j + 1
                cf[j] = np.log(np.abs(t1) + 1)**k + np.log(np.abs(t2) + 1)**(35 - k) + np.real(t1 * t2) * np.imag(t1 + t2)
        # Assign specific intricate coefficients
        cf[[4, 9, 14, 19, 24, 29, 34]] = [2 + 3j, -4j, 5 - 6j, -7 + 8j, 9 - 10j, 11 + 12j, -13 + 14j]
        # More intricate assignments
        cf[7] = 100j * t2**3 + 100j * t2**2 - 100 * t2 - 100
        cf[11] = 150j * t1**3 + 150j * t1**2 + 150 * t2 - 150
        cf[17] = 200j * t2**3 - 200j * t2**2 + 200 * t2 - 200
        cf[21] = 250 * np.sin(t1) + 300j * np.cos(t2) + 50 * np.log(np.abs(t1) + 1)
        cf[27] = 350 * np.prod([t1, t2]) + 400j * np.sum([t1, t2])
        cf[32] = 450j * t1 * t2 + 500 * np.conj(t1 - t2)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_225(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        # Fixed coefficients with varied complex values
        fixed_indices = [3, 8, 14, 19, 23, 29]
        fixed_values = [2 - 1j, -3 + 4j, 5 - 2j, -4 + 3j, 1.5 - 0.5j, 3 + 2j]
        cf[fixed_indices] = fixed_values
        
        # Loop to assign coefficients with intricate patterns
        for j in range(35):
            if j not in fixed_indices:
                k = (j % 7) + 1
                r = np.real(t1) * np.imag(t2) / (k + j + 1)
                angle = np.angle(t1) + np.angle(t2) * k
                magnitude = np.abs(t1)**k + np.abs(t2)**(7 - k)
                cf[j] = (magnitude * np.cos(angle)) + (r * np.sin(angle)) + np.conj(t1) * t2 / (j + 1)
        
        # Additional intricate assignments
        for k in range(1, 6):
            idx = 5 * k
            if idx <= 35:
                cf[idx] = (np.real(t1)**k - np.imag(t2)**k) * np.cos(k * np.angle(t1)) + (np.abs(t1) + np.abs(t2)) * np.sin(k * np.angle(t2)) + np.conj(t1)**k * np.conj(t2)**k / (k + 1)
        
        # Define specific coefficients with creative combinations
        cf[9] = np.log(np.abs(t1 * t2) + 1) * (np.real(t1) + np.imag(t2)) + 2j * np.real(t1)**2 - 3 * np.imag(t2)**2
        cf[20] = np.conj(t1) * t2**3 + np.real(t2) * np.imag(t1) - 4j * np.abs(t1 + t2)
        cf[35] = np.sum(np.real(cf)) + np.sum(np.imag(cf)) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_226(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        # Assign fixed coefficients
        cf[[0, 6, 13, 20, 27, 34]] = [2, -1 + 3j, 4 - 2j, -3 + 5j, 1.5 - 1.5j, 0.3 + 0.7j]
        
        # Loop to assign other coefficients with intricate calculations
        for j in range(2, 35):
            if j not in [0, 6, 13, 20, 27, 34]:
                r = np.real(t1) + np.imag(t2) * (j + 1)
                theta = np.angle(t1) * (j + 1) - np.angle(t2) / ((j + 1) % 5 + 1)
                magnitude = np.sin(r) * np.cos(r + theta) + np.log(np.abs(t1) + np.abs(t2) + (j + 1))
                phase = theta + np.sin((j + 1) * np.real(t1)) - np.cos((j + 1) * np.imag(t2))
                cf[j] = magnitude * (np.cos(phase) + 1j * np.sin(phase))
        
        # Additional intricate assignments
        cf[9] = np.conj(t1) * t2**2 + np.sin(t1 * t2)
        cf[18] = np.abs(t1 + t2) * np.exp(1j * np.angle(t1 - t2))
        cf[25] = np.sum([np.real(t1), np.imag(t1), np.real(t2), np.imag(t2)]) + np.prod([np.abs(t1), np.abs(t2)])
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_227(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(35):
            k = (j + 1) + (j // 5)
            magnitude = np.log(np.abs(t1) + 1) * np.sin(j + 1) + np.log(np.abs(t2) + 1) * np.cos(j + 1)
            angle = np.angle(t1) * (j + 1)**0.5 - np.angle(t2) * np.log(j + 2)
            cf[j] = magnitude * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1)**k - np.conj(t2)**(35 - j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_228(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        # Assign fixed coefficients
        cf[[3, 7, 13, 17, 26, 32]] = [2.5, -3.4, 5.6, -4.2, 3.1, 0.8]
        # Loop through coefficients to assign intricate patterns
        for j in range(35):
            if j not in [3, 7, 13, 17, 26, 32]:
                k = 35 - j
                r = (j % 5) + 1
                angle = np.angle(t1) * (j + 1) + np.angle(t2) * k
                magnitude = np.log(np.abs(t1) + 1) * np.sin(j + 1) + np.log(np.abs(t2) + 1) * np.cos(k)
                cf[j] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        
        # Additional intricate coefficient assignments
        cf[9] = np.conj(t1)**2 * t2 + np.sin(t1 * t2)
        cf[18] = np.abs(t1 + t2) * np.exp(1j * np.angle(t1 - t2))
        cf[25] = np.sum([np.real(t1), np.imag(t1), np.real(t2), np.imag(t2)]) + np.prod([np.abs(t1), np.abs(t2)])
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_229(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        # Initialize specific coefficients with unique values
        cf[[2, 6, 13, 18, 21, 27]] = [2 - 3j, -4 + 5j, 1.5 - 2.5j, -3.3 + 4.4j, 0.5 - 1.2j, 3 - 3j]
        
        # Loop through coefficients and assign values based on intricate calculations
        for j in range(35):
            if j not in [2, 6, 13, 18, 21, 27]:
                angle = np.angle(t1**(j + 1) + t2**(35 - j))
                magnitude = np.abs(t1)**((j % 5) + 1) * np.abs(t2)**((35 - j) % 7 + 1)
                cf[j] = magnitude * (np.cos(angle) + np.sin(angle) * 1j) + np.conj(t1**2 - t2**2)
        
        j = 35
        # Additional intricate computations for specific coefficients
        cf[9] = np.sin(t1 * j) + np.cos(t2 * j) + np.log(np.abs(t1) + np.abs(t2) + 1)
        cf[17] = np.real(t1)**2 - np.imag(t2)**2 + 2j * np.real(t1) * np.imag(t2)
        cf[25] = np.prod([np.abs(t1), np.abs(t2)]) * np.exp(1j * np.angle(t1 + t2))
        cf[30] = np.sum([np.abs(t1 + t2), np.abs(t1 - t2)]) + 1j * np.sum([np.angle(t1), np.angle(t2)])
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_230(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        # Assign fixed coefficients
        cf[[2, 7, 11, 16, 22, 27, 31, 34]] = [3, -2, 5, -4, 6, -3, 2, -1]
        
        # Loop to assign coefficients with intricate patterns
        for j in range(35):
            if (j + 1) % 4 == 0:
                cf[j] = (np.real(t1)**2 + np.imag(t2)) * (j + 1) + (np.imag(t1) * np.real(t2)) * 1j
            elif (j + 1) % 5 == 1:
                cf[j] = np.sin(t1 * (j + 1)) + np.cos(t2 + (j + 1)) * 1j
            elif (j + 1) % 3 == 2:
                cf[j] = np.log(np.abs(t1) * (j + 1) + 1) + np.angle(t2)**(j + 1) * 1j
            else:
                cf[j] = np.real(t1 + t2) * (j + 1) + np.imag(t1 - t2) * 1j
        
        # Additional intricate assignments using nested loops
        for k in range(1, 6):
            for r in range(1, 8):
                idx = (k * r) % 35
                cf[idx] += (np.real(t1)**k * np.imag(t2)**r) + (np.real(t2)**k * np.imag(t1)**r) * 1j
        
        # Modify certain coefficients with conjugates and products
        for m in range(10, 31, 5):
            cf[m] = cf[m] * np.conj(t1) + np.prod([np.abs(t2), m]) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_231(t1, t2):
    try:
        cf = np.zeros(50, dtype=np.complex128)
        for k in range(50):
            magnitude = np.abs(t1)**((k % 5) + 1) + np.abs(t2)**((k % 7) + 1) + np.log(np.abs(t1) + 1) * np.sin(k + 1)
            angle = np.angle(t1) * np.cos(k + 1) + np.angle(t2) * np.sin(k + 1)
            cf[k] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        for j in range(2, 51, 3):
            cf[j] += np.conj(t1) * t2**(j % 4)
        for r in range(5, 51, 5):
            cf[r] += np.real(t2) * np.cos(r) + np.imag(t1) * np.sin(r) * 1j
        cf[9] = np.sum([np.abs(t1), np.abs(t2)]) + np.prod([np.real(t1), np.real(t2)]) * 1j
        cf[19] = np.real(t1)**2 - np.imag(t2)**2 + 2 * np.real(t1) * np.imag(t2) * 1j
        cf[29] = np.log(np.abs(t1) + np.abs(t2) + 1) * (np.sin(np.angle(t1)) + np.cos(np.angle(t2)) * 1j)
        cf[39] = np.abs(t1) * np.abs(t2) * np.exp(1j * (np.angle(t1) - np.angle(t2)))
        cf[49] = np.conj(t1) + np.conj(t2) - t1 * t2 * 1j
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_232(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = j % 5 + 1
            r = j // 7 + 1
            cf[j - 1] = (np.real(t1)**k - np.imag(t2)**r) * np.sin(j * np.angle(t1 + t2)) / (np.abs(t1) + np.abs(t2) + 1) + \
                np.conj(t1)**k * np.cos(r * np.angle(t2)) + np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1)
        
        cf[[2, 7, 11, 18, 22, 28, 33]] = np.array([
            (t1 * t2)**2 - np.conj(t1) * np.sin(t2),
            np.abs(t1) * np.real(t2) + np.imag(t1) * np.imag(t2),
            np.cos(t1) + np.sin(t2),
            np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1),
            t1**3 - t2**3 + np.conj(t1 * t2),
            np.real(t1 + t2) * np.imag(t1 - t2),
            np.sin(t1 * t2) + np.cos(np.conj(t1)) * np.cos(np.conj(t2))
        ])
        
        cf[[4, 9, 14, 19, 24, 29]] = np.array([
            np.real(t1) ** 2 + np.imag(t2) ** 2,
            np.angle(t1) * np.angle(t2),
            np.abs(t1 + t2) * np.conj(t1 - t2),
            np.sin(np.abs(t1)) * np.cos(np.abs(t2)),
            np.log(np.abs(t1 * t2) + 1),
            np.sum([np.real(t1), np.imag(t2), np.angle(t1 + t2)])
        ])
        
        cf[[6, 10, 16, 20, 26, 30]] = np.array([
            t1**2 * t2 - t1 * t2**2,
            np.conj(t1)**2 + np.conj(t2)**2,
            np.sin(t1) * np.cos(t2) + np.cos(t1) * np.sin(t2),
            np.real(t1 * t2) + np.imag(t1 * t2),
            (t1 + t2)**3 - (t1 - t2)**3,
            np.prod([np.abs(t1), np.abs(t2), np.real(t1 + t2)])
        ])
        
        cf[[8, 12, 17, 23, 27, 31]] = np.array([
            np.real(t1)**3 - np.imag(t2)**3,
            np.angle(t1)**2 + np.angle(t2)**2,
            np.sin(t1 + t2) - np.cos(t1 - t2),
            np.log(np.abs(t1)**2 + np.abs(t2)**2 + 1),
            np.real(np.conj(t1) * t2),
            np.imag(t1 * np.conj(t2))
        ])
        
        cf[[3, 5, 15, 20, 21, 25, 32, 34]] = np.array([
            np.real(t1) * np.real(t2),
            np.imag(t1) * np.imag(t2),
            np.angle(t1 + t2) * np.abs(t1 * t2),
            np.sin(np.real(t1)) + np.cos(np.imag(t2)),
            np.log(np.abs(t1 + t2) + 1),
            np.real(np.conj(t1 + t2)),
            np.sin(np.abs(t1)**2) * np.cos(np.abs(t2)**2),
            np.real(t1)**2 + np.imag(t1)**2 + np.real(t2)**2 + np.imag(t2)**2
        ])
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_233(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        # Assign initial coefficients with direct values
        cf[[2, 5, 11, 17, 23, 29]] = np.array([2 - 3j, -4 + 2j, 5 - 1j, -6 + 3j, 7 - 2j, -8 + 4j])
        
        # Loop to assign intricate coefficients
        for j in range(1, 36):
            if j % 4 == 0:
                cf[j - 1] = (np.real(t1) * j**2 - np.imag(t2) * j) + (np.imag(t1) + np.real(t2)) * 1j
            elif j % 5 == 0:
                cf[j - 1] = (np.abs(t1) ** (j % 3 + 1)) * np.cos(np.angle(t2) * j) + (np.sin(np.angle(t1) * j) * np.abs(t2)) * 1j
            elif j % 3 == 0:
                cf[j - 1] = np.log(np.abs(t1) + 1) * j - np.log(np.abs(t2) + 1) * j * 1j
            else:
                cf[j - 1] = (np.real(t1) + np.real(t2)) * j + (np.imag(t1) - np.imag(t2)) * j * 1j
        
        # Introduce variations with complex operations
        for k in range(1, 36):
            if k % 7 == 0:
                cf[k - 1] = (np.conj(t1) * t2**2) + (np.sin(t1) - np.cos(t2)) * 1j
            if k % 11 == 0:
                cf[k - 1] = np.prod([np.real(t1), np.imag(t2), k]) + np.sum([np.abs(t1), np.abs(t2), k]) * 1j
        
        # Assign specific coefficients to ensure non-symmetry
        cf[4] = 10 * t1 - 5j * t2**2
        cf[9] = 15j * t1**3 + 8 * t2
        cf[14] = 20 * t1**2 - 10j * t2**3
        cf[19] = 25j * t1 - 12 * t2**2
        cf[24] = 30 * t1**4 + 15j * t2
        cf[34] = 35j * t1**2 - 18 * t2**3
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_234(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = j + 2
            r = np.sqrt(j)
            mag = (np.abs(t1)**r + np.abs(t2)**(k % 5 + 1)) * np.sin(j) + np.log(np.abs(t1) + 1) * np.cos(r)
            ang = np.angle(t1) * np.cos(j / 2) - np.angle(t2) * np.sin(r)
            cf[j - 1] = mag * (np.cos(ang) + np.sin(ang) * 1j)
            cf[j - 1] += np.conj(t1) * t2**k - np.conj(t2) * t1**(k % 3)
        
        cf[4] = np.real(t1) + np.imag(t2) * 1j
        cf[11] = np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1) + np.sin(t1 * t2)
        cf[19] = (np.abs(t1) - np.abs(t2)) * np.cos(np.angle(t1) - np.angle(t2)) + 2j * np.sin(np.angle(t1) + np.angle(t2))
        cf[24] = np.conj(t1 + t2) * (np.real(t1) - np.imag(t2)) + 3j
        cf[29] = np.sum([np.abs(t1), np.abs(t2)]) + np.prod([np.abs(t1), np.abs(t2)]) * 1j
        cf[34] = np.angle(t1 * t2) + np.abs(t1 + t2) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_235(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        # Fixed coefficients
        cf[[2, 6, 11, 18, 24, 29]] = np.array([3, -5, 7, -11, 13, -17])
        
        # Loop for first 10 coefficients
        for j in range(1, 11):
            magnitude = np.log(np.abs(t1)**j + 1) + np.abs(t2)**(j % 3 + 1)
            angle = np.angle(t1) * j - np.angle(t2) * (j % 2)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        
        # Loop for coefficients 11 to 20
        for k in range(11, 21):
            magnitude = np.sin(np.real(t1) * k) + np.cos(np.imag(t2) * k)
            angle = np.log(np.abs(t1 + t2) + 1) * k
            cf[k - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        
        # Loop for coefficients 21 to 34
        for r in range(21, 35):
            magnitude = np.log(np.abs(t1 * r) + 1) + np.real(t2)**2
            angle = np.angle(np.conj(t1) + np.conj(t2)) * r
            cf[r - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        
        # Last coefficient
        cf[34] = np.sin(t1 * t2) + np.cos(t1 / (t2 + 1)) + 1j * np.log(np.abs(t1 + t2) + 1)
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_236(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            angle = np.sin(np.real(t1) * j) * np.cos(np.imag(t2) * j) + np.angle(t1 + t2) / j
            magnitude = np.log(np.abs(t1) * j + np.abs(t2) + 1) * (1 + np.sin(j))**0.5
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        
        k = 1
        while k <= 35:
            if k % 5 == 0:
                cf[k - 1] = cf[k - 1] * np.conj(t1) + np.abs(t2)**2
            elif k % 3 == 0:
                cf[k - 1] += t1**k - t2**k
            else:
                cf[k - 1] += np.sin(t1 * k) * np.cos(t2 * k)
            k += 1
        
        r = 2
        for r in range(2, 6):
            idx = r**2
            if idx <= 35:
                cf[idx - 1] += np.prod([np.real(t1), np.imag(t2)]) / r
        
        cf[[3, 9, 15, 21, 27, 33]] += 100j * (t1**2 - t2**2) / (r + 1)
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_237(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 4 == 1:
                cf[j - 1] = (t1**j * np.sin(j * np.angle(t2))) + np.conj(t2)**2
            elif j % 4 == 2:
                cf[j - 1] = (t2**j * np.cos(j * np.angle(t1))) + np.conj(t1)**2
            elif j % 4 == 3:
                cf[j - 1] = np.real(t1) * np.imag(t2) * np.log(np.abs(t1) + 1) + np.real(t2)**j
            else:
                cf[j - 1] = np.imag(t1) * np.real(t2) * np.log(np.abs(t2) + 1) + np.imag(t2)**j
            
            cf[j - 1] += (np.conj(t1) * np.conj(t2)) / (j + 1)
        
        for k in range(1, 36):
            if k % 5 == 0:
                cf[k - 1] = cf[k - 1] * (1 + 0.05 * k) + np.sin(k * np.angle(cf[k - 1]))
            else:
                cf[k - 1] = cf[k - 1] / (1 + 0.02 * k) + np.cos(k * np.angle(cf[k - 1]))
            cf[k - 1] += np.log(np.abs(cf[k - 1]) + 1) * np.real(cf[k - 1])
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_238(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            angle = np.angle(t1) * j**2 - np.angle(t2) * np.sqrt(j)
            magnitude = np.abs(t1)**j + np.abs(t2)**(35 - j) + np.log(np.abs(t1) + np.abs(t2) + j)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        
        for k in range(1, 36):
            r = np.sqrt(k)
            cf[k - 1] += np.conj(cf[36 - k]) * t1**(k % 5) - np.conj(cf[k - 1]) * t2**(35 - k % 3)
        
        cf[4] += 2 * t1**3 - 3 * t2**2 + np.sin(t1 * t2) * 1j
        cf[9] = np.conj(cf[9]) * t1 - np.imag(cf[9]) * t2 + np.log(np.abs(t1 + t2) + 1)
        cf[14] = cf[14] * t1**2 - cf[14] / (np.abs(t2) + 1) + np.cos(t1 - t2) * 1j
        cf[19] = np.real(cf[19]) + np.imag(cf[19]) * 1j + t1 * t2
        cf[24] = np.abs(t1) * np.abs(t2) + np.angle(t1 + t2) * 1j
        cf[29] = np.sin(t1**2) + np.cos(t2**3) * 1j - np.log(np.abs(t1 * t2) + 1)
        cf[34] = np.conj(cf[34]) + t1 - t2 + np.sin(t1 + t2) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_239(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 5 == 1:
                cf[j - 1] = np.real(t1)**j + np.imag(t2)**2
            elif j % 5 == 2:
                cf[j - 1] = np.abs(t1) * np.abs(t2)**j * np.exp(1j * np.angle(t1) * j)
            elif j % 5 == 3:
                cf[j - 1] = np.conj(t1) * np.sin(t2)**j + np.cos(t1 * t2)
            elif j % 5 == 4:
                cf[j - 1] = np.log(np.abs(t1) + 1) * (t2**j) + 1j * np.log(np.abs(t2) + 1)
            else:
                cf[j - 1] = (t1 + t2)**j - (t1 - t2)**j
        
        # Additional intricate assignments
        cf[4] += 2j * t1 * t2
        cf[9] = np.real(t1)**2 - np.imag(t2)**3 + 3j * np.abs(t1 * t2)
        cf[14] = np.sin(t1 + t2) * np.cos(t1 - t2) + 1j * np.log(np.abs(t1) + np.abs(t2) + 1)
        cf[19] = (t1 * t2)**2 - np.conj(t1) * np.conj(t2) + 2j * np.angle(t1 + t2)
        cf[24] = np.real(t1 * t2) + np.imag(t1)**2 - np.imag(t2)**2 + 1j * (np.real(t1) - np.real(t2))
        cf[29] = np.abs(t1 + t2)**3 * np.exp(1j * np.angle(t1 - t2))
        cf[34] = np.sin(np.abs(t1) * t2) + np.cos(np.abs(t2) * t1) + 1j * (np.real(t1) * np.real(t2))
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_240(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        # Assign specific coefficients
        cf[[0, 4, 12, 19, 20, 24]] = np.array([1 + 0j, 4 + 0j, 4 + 0j, -9 + 0j, -1.9 + 0j, 0.2 + 0j])
        
        # Assign coefficients with intricate patterns
        for j in range(2, 35):
            if j not in [5, 13, 20, 21, 25]:
                mag = np.log(np.abs(t1 + j) + 1) * np.sin(j * np.angle(t2)) + np.cos(j * np.angle(t1))
                angle = np.angle(t1)**j + np.sin(j * np.angle(t2)) - np.cos(j)
                cf[j - 1] = mag * np.cos(angle) + mag * np.sin(angle) * 1j
        
        # Assign the last coefficient with a unique combination
        cf[34] = np.conj(t1) * np.conj(t2) + np.sin(np.abs(t1) * np.abs(t2)) + np.log(np.abs(t1) + np.abs(t2) + 1) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_241(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (np.abs(t1)**np.sin(j) + np.abs(t2)**np.cos(j))
            angle = np.angle(t1) * j - np.angle(t2) * (35 - j) + np.sin(j) * np.cos(j)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        
        for k in range(1, 36):
            cf[k - 1] += np.conj(t1) * t2**k / (k + 1)
        
        cf[[4, 9, 14, 19, 24, 29]] += 50 * (np.real(t1) - np.imag(t2)) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_242(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 5 == 1:
                cf[j - 1] = np.sin(np.abs(t1) * j) + np.cos(np.angle(t2) * j)
            elif j % 5 == 2:
                cf[j - 1] = np.log(np.abs(t1) + 1) * t2**j
            elif j % 5 == 3:
                cf[j - 1] = np.conj(t1)**j - np.real(t2) * j
            elif j % 5 == 4:
                cf[j - 1] = np.imag(t1) + np.abs(t2) * np.sin(j * np.angle(t1))
            else:
                cf[j - 1] = t1 * t2**j + np.cos(j) - np.sin(j)
        
        cf[6] = 50j * t1**2 - 30j * t2 + 20
        cf[13] = 80 * t1 - 60j * t2**2 + 10
        cf[20] = 40j * t1**3 + 25 * np.conj(t2) - 15
        cf[27] = 70 * np.abs(t1) + 35j * np.angle(t2) + 5
        cf[34] = 90j * t1 * t2 - 45 * np.real(t1) + 22.5
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_243(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        # Assign base coefficients with fixed values
        cf[[0, 5, 9, 14, 21, 27]] = np.array([2, -3 + 2j, 4.5, -5.2j, 3.3, -1.1])
        
        # Loop to assign lower degree coefficients
        for j in range(2, 6):
            cf[j - 1] = (np.real(t1)**j + np.imag(t2)**j) * np.sin(np.angle(t1) * j) / (1 + j)
        
        # Loop to assign middle degree coefficients
        for k in range(7, 15):
            cf[k - 1] = (np.abs(t1)**k * np.cos(np.angle(t2) * k)) + np.conj(t2) * np.log(np.abs(t1 * t2) + 1)
        
        # Loop to assign higher degree coefficients
        for r in range(16, 26):
            cf[r - 1] = (np.real(t1**r) - np.imag(t2**r) * 1j) * np.sin(t1 + t2) + np.cos(t1 * t2)
        
        # Assign coefficients using product and sum
        cf[25] = np.prod([np.abs(t1), np.abs(t2)]) + np.sum([np.real(t1), np.imag(t2)]) * np.conj(t1 + t2)
        cf[26] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1) * 1j
        cf[28] = np.real(t1 * t2) - np.imag(t1 / t2) * 1j
        cf[29] = np.sin(t1**2) + np.cos(t2**3) * 1j
        cf[31] = np.abs(t1 + t2) * np.exp(-np.real(t1 - t2))
        cf[33] = np.angle(t1) + np.angle(t2) * 1j
        
        # Assign the last coefficient with a unique pattern
        cf[34] = (t1**3 + t2**3) / (1 + np.abs(t1) + np.abs(t2))
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_244(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = (j % 6) + 1
            r = (j % 4) + 1
            angle_part = np.sin(j * np.real(t1)) * np.cos(j * np.imag(t2)) + np.angle(t1) / (k + 1)
            mag_part = np.abs(t1)**k * np.abs(t2)**r + np.log(np.abs(t1) + np.abs(t2) + j)
            cf[j - 1] = np.cos(angle_part) * mag_part + np.sin(angle_part) * mag_part * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_245(t1, t2):
    try:
        # Initialize complex coefficient vector of length 35
        cf = np.zeros(35, dtype=np.complex128)
        
        # Assign fixed coefficients with non-symmetric values
        cf[[1, 5, 9, 13, 17, 21, 25, 29, 33]] = np.array([2 + 3j, -3 + 2j, 4 - 1j, -5 + 4j, 
                                                              6 - 3j, -7 + 5j, 8 - 4j, -9 + 6j, 10 - 5j])
        
        # Assign coefficients using loop for j indices with intricate calculations
        j_indices = [0, 4, 8, 12, 16, 20, 24, 28, 32]
        for j in j_indices:
            cf[j] = (np.real(t1) * np.imag(t2) + np.imag(t1) * np.real(t2)) + \
                     (np.abs(t1)**2 - np.abs(t2)**2) * np.sin(t1 + t2) + \
                     np.log(np.abs(t1) + 1) * np.cos(t2)
        
        # Assign coefficients using loop for k indices with complex functions
        k_indices = [2, 6, 10, 14, 18, 22, 26, 30, 34]
        for k in k_indices:
            cf[k] = np.sin(t1 * t2) + np.cos(t1 / (np.abs(t2) + 1)) * np.conj(t2) + \
                     np.angle(t1 + t2) * np.abs(t1 - t2) + \
                     np.prod([np.real(t1), np.imag(t2)])
        
        # Assign coefficients using loop for r indices with mixed parameters
        r_indices = [3, 7, 11, 15, 19, 23, 27, 31]
        for r in r_indices:
            cf[r] = np.real(t1)**3 - np.imag(t2)**3 + \
                     np.real(t1 * t2) + np.imag(t1 + t2) + \
                     np.log(np.abs(t1 * t2) + 1)
        
        # Additional intricate assignments for specific coefficients
        cf[18] = 100j * t1**3 + 50j * t2**2 - 75 * t1 * t2 + 25
        cf[22] = 80j * t2**3 - 60j * t1**2 + 40 * np.sin(t1 + t2) - 20
        cf[26] = 90j * t1 * t2**2 - 70 * np.cos(t1) + 50 * np.log(np.abs(t2) + 1)
        cf[30] = 110j * np.sin(t1**2) - 95 * np.abs(t2) * t1 + 85j * np.angle(t1 + t2)
        cf[34] = 120j * np.cos(t1 * t2) - 100 * np.sin(t2) + 75 * np.log(np.abs(t1) + 1)
        
        # Return the complex coefficient vector
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_246(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        # Fixed coefficients with varied complex expressions
        cf[[0, 3, 7, 11, 15, 19, 23, 27, 31]] = np.array([
            2 + 3j,
            np.conj(t1) * np.sin(t2),
            np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1) * 1j,
            np.real(t1)**2 - np.imag(t2)**2 + (np.real(t2) * np.imag(t1)) * 1j,
            np.sin(t1 * t2) + np.cos(t1 + t2) * 1j,
            np.prod([t1, t2]) + np.sum([np.real(t1), np.imag(t2)]) * 1j,
            np.abs(t1)**3 - np.abs(t2)**3 + np.angle(t1) * np.angle(t2) * 1j,
            np.real(t2) * np.sin(np.angle(t1)) + np.imag(t1) * np.cos(np.angle(t2)) * 1j,
            np.real(t1 + t2) + np.imag(t1 - t2) * 1j
        ])
        # Loop to assign remaining coefficients with intricate patterns
        for j in [2, 3, 5, 6, 8, 9, 10, 12, 13, 14, 16, 17, 18, 20, 21, 22, 24, 25, 26, 27, 28, 29, 30, 32, 33, 34]:
            k = j * 3
            r = j % 4
            cf[j] = (np.real(t1)**k + np.imag(t2)**k) * np.sin(k * np.angle(t1)) + \
                     (np.real(t2)**r - np.imag(t1)**r) * np.cos(r * np.angle(t2)) * 1j + \
                     np.log(np.abs(t1) + np.abs(t2) + j) * (1 + 1j)
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_247(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            angle = np.angle(t1)**j + np.angle(t2)**(j % 5 + 1)
            magnitude = np.abs(t1)**(j % 7) * np.abs(t2)**(j // 5 + 1)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        
        k = 1
        while k <= 35:
            cf[k - 1] += np.conj(t1) * t2**(k % 3) - np.log(np.abs(t1 + t2) + 1)
            k += 4
        
        for r in range(2, 35, 3):
            cf[r - 1] *= (np.sin(t1 * r) + np.cos(t2 / (r + 1)))
        
        cf[9] = np.sum([np.abs(t1), np.abs(t2)]) * np.exp(1j * np.angle(t1 + t2))
        cf[19] = np.prod([np.abs(t1), np.abs(t2)]) / (1 + np.abs(t1 - t2))
        cf[34] = np.real(t1)**3 - np.imag(t2)**2 + 2j * np.real(t2) * np.imag(t1)
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_248(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        # Initialize specific coefficients
        cf[[1, 6, 12, 18, 24, 30]] = np.array([3, -5, 8, -12, 20, -25])
        
        # Loop to assign coefficients with intricate patterns
        for j in range(1, 36):
            if j % 4 == 1:
                angle = np.angle(t1) * j + np.sin(j * np.angle(t2))
                magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (j % 3 + 1)
                cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
            elif j % 4 == 2:
                angle = np.angle(t2) * j + np.cos(j * np.angle(t1))
                magnitude = np.abs(t1)**2 + np.abs(t2)**2 + j
                cf[j - 1] = magnitude * (np.cos(angle) - 1j * np.sin(angle))
            elif j % 4 == 3:
                angle = np.sin(j * np.angle(t1 + t2))
                magnitude = np.log(np.abs(t1 * t2) + 1) * (j + 2)
                cf[j - 1] = magnitude * np.exp(1j * angle)
            else:
                angle = np.cos(j * np.angle(t1 - t2))
                magnitude = (np.abs(t1) + np.abs(t2))**j / (j + 1)
                cf[j - 1] = magnitude * (1 + 1j * angle)
        
        # Additional intricate modifications
        for k in range(5, 36, 5):
            cf[k - 1] += (np.real(t1)**k - np.imag(t2)**k) * 1j
        
        for r in range(10, 16):
            cf[r - 1] *= (1 + 0.5j * np.real(t1 + t2))
        
        # Assign non-symmetric, non-circular roots patterns
        cf[19] = np.prod(np.abs(cf[0:10]))**(1/5) * (np.sin(np.angle(t1)) + np.cos(np.angle(t2)))
        cf[33] = np.conj(cf[33]) + t1**3 - t2**3
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_249(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 5 == 1:
                cf[j - 1] = np.real(t1)**j + np.imag(t2)**(j % 3) * np.sin(j * np.angle(t1))
            elif j % 5 == 2:
                cf[j - 1] = np.conj(t1) * np.cos(j * np.angle(t2)) + np.abs(t2)**2 / (j + 1)
            elif j % 5 == 3:
                cf[j - 1] = np.log(np.abs(t1) + 1) + 1j * np.log(np.abs(t2) + 1) + np.real(t1)**2 - np.imag(t2)**2
            elif j % 5 == 4:
                cf[j - 1] = (np.real(t1) * np.imag(t2))**j + (np.abs(t1) + np.abs(t2)) * np.sin(j)
            else:
                cf[j - 1] = np.sum([np.real(t1), np.imag(t2)]) * np.cos(j * np.angle(t1) * np.angle(t2)) + 1j * np.prod([np.abs(t1), np.abs(t2)])
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_250(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 4 == 1:
                cf[j - 1] = (np.real(t1)**j + np.sin(j * np.angle(t1))) + 1j * (np.imag(t1)**j + np.cos(j * np.abs(t1)))
            elif j % 4 == 2:
                cf[j - 1] = np.log(np.abs(t2) + 1) * (np.real(t2)**j - np.imag(t2)**j) + 1j * (np.angle(t2)**j + np.abs(t2)**j)
            elif j % 4 == 3:
                cf[j - 1] = np.sin(t1 * j) * np.cos(t2 * j) + np.conj(t1) * np.conj(t2)
            else:
                cf[j - 1] = np.abs(t1 + t2)**j + 1j * np.angle(t1 - t2)
        
        for k in range(1, 8):
            idx = k * 5
            if idx <= 35:
                cf[idx - 1] *= (np.sin(k) + 1j * np.cos(k))
        
        cf[7] = np.sum(np.abs(cf[0:7])) + 1j * np.prod(np.abs(cf[0:7]))
        cf[15] = np.cos(t1 + t2) + 1j * np.sin(t1 - t2)
        cf[23] = np.log(np.abs(t1**2 - t2**2) + 1) + 1j * np.angle(t1 * t2)
        cf[31] = np.conj(t1)**3 + np.conj(t2)**2 + np.sin(t1 * t2)
        cf[34] = np.real(t1) * np.real(t2) + np.imag(t1) * np.imag(t2) + 1j * (np.real(t1) - np.imag(t2))
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_251(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            r = np.real(t1) + np.real(t2) + j
            angle = np.angle(t1) * j - np.angle(t2)
            cf[j - 1] = (np.abs(t1)**j + np.abs(t2)**(35 - j)) * np.exp(1j * angle) * np.sin(j * np.real(t1) - np.imag(t2))
        
        cf[4] = np.conj(t1) * t2**2 - np.log(np.abs(t1) + 1) + 2j * np.real(t2)
        cf[9] = np.sin(t1) + np.cos(t2) * np.conj(t1)
        cf[14] = (t1 * t2)**3 - np.real(t1)**2 + np.imag(t2)**3
        cf[19] = np.exp(1j * np.angle(t1)) * np.log(np.abs(t2) + 1) + np.abs(t1 + t2)
        cf[24] = np.sin(t1 + t2) * np.cos(t1 - t2) + 1j * (np.real(t1) * np.imag(t2))
        cf[29] = np.prod([np.real(t1), np.imag(t2), np.abs(t1 + t2)]) + np.sum([np.real(t2), np.imag(t1)])
        cf[34] = np.conj(t1)**2 + np.conj(t2)**3 - t1 * t2
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_252(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = j % 6
            r = j // 6 + 1
            if k == 1:
                cf[j - 1] = (np.log(np.abs(t1) + 1) + np.sin(np.angle(t2))) * t1**r
            elif k == 2:
                cf[j - 1] = (np.cos(np.angle(t1)) - np.sin(np.abs(t2))) * np.conj(t2)**r
            elif k == 3:
                cf[j - 1] = (np.real(t1) * np.imag(t2) + np.real(t2) * np.imag(t1)) * (t1 + t2)**r
            elif k == 4:
                cf[j - 1] = (np.abs(t1)**2 - np.abs(t2)**2) * np.exp(1j * np.angle(t1 * t2)) * r
            elif k == 5:
                cf[j - 1] = (np.sin(t1 * r) + np.cos(t2 / r)) * (t1 - t2)**2
            else:
                cf[j - 1] = (np.log(np.abs(t1 * t2) + 1) + np.angle(t1 + t2)) * (t1 + np.conj(t2))**r
        
        cf[4] = 100j * t1**4 - 50 * t2**2 + 25j
        cf[11] = 75 * np.conj(t1) - 60j * t2 + 30
        cf[18] = (t1**3 + t2**3) / (np.real(t1) + np.real(t2) + 1)
        cf[25] = np.sin(t1 + t2) * np.cos(t1 - t2) * 1j
        cf[32] = np.log(np.abs(t1 + t2) + 1) * (t1**2 - t2**2)
        cf[34] = np.real(t1 * t2) + np.imag(t1 - t2) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_253(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 4 == 0:
                k = j // 4
                angle = np.angle(t1 + t2) * k
                cf[j - 1] = (np.real(t1)**k + np.imag(t2)**k) * (np.cos(angle) + np.sin(angle) * 1j)
            elif j % 5 == 0:
                r = j // 5
                cf[j - 1] = np.log(np.abs(t1) * r + 1) + np.conj(t2)**r
            elif j % 3 == 1:
                cf[j - 1] = np.sin(t1 * j) + np.cos(t2 * j) * 1j
            else:
                cf[j - 1] = np.real(t1 * t2) + np.imag(t1 / t2) * 1j
        
        cf[6] = np.prod([np.real(t1), np.imag(t2)]) + np.sum([np.abs(t1), np.abs(t2)]) * 1j
        cf[13] = t1**3 + t2**2 - 5 * t1 * t2 * 1j
        cf[20] = np.sin(t1 + t2) + np.cos(t1 - t2) * 1j
        cf[27] = np.log(np.abs(t1) + 1) * np.conj(t2) - np.sin(t1 * t2)
        cf[34] = np.real(t1)**2 - np.imag(t2)**2 + 2 * np.real(t1) * np.imag(t2) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_254(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = j % 5 + 1
            r = j // 5 + 1
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude = np.abs(t1)**k + np.abs(t2)**r + np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        
        cf[[2, 7, 13, 21, 28]] = np.conj(t1) * t2**2 - t1**2 * np.conj(t2)
        cf[[4, 10, 18, 26, 34]] = np.sin(t1 * t2) + np.cos(t1 + t2) * 1j
        cf[16] = np.prod([np.abs(t1), np.abs(t2)]) * np.exp(1j * (np.angle(t1) - np.angle(t2)))
        cf[24] = np.sum([np.abs(t1 + t2), np.real(t1)**2, np.imag(t2)**2]) * (1 + 1j)
        cf[34] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_255(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        # Initialize specific coefficients with fixed values
        cf[[2, 7, 13, 18, 25, 33]] = [2 + 3j, -4 + 1j, 5 - 2j, -3 + 4j, 1.5 - 0.5j, -2.2 + 2j]
        
        # Loop to assign intricate coefficients
        for j in range(1, 36):
            if j not in [3, 8, 14, 19, 26, 34]:
                k = j % 7 + 1
                r = j // 5 + 1
                magnitude = np.sin(j * np.angle(t1)) * np.cos(k * np.abs(t2)) + np.log(np.abs(t1) + 1) * r
                angle = np.angle(t2) * k - np.angle(t1) * r + np.sin(j * np.imag(t1))
                cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)

        # Additional intricate patterns
        for k in range(1, 6):
            idx = 7 * k
            if idx <= 35:
                cf[idx - 1] = (np.conj(t1)**k + t2**k) * np.exp(-k / (np.abs(t1) + np.abs(t2) + 1)) + \
                               (np.sin(t1 * k) + np.cos(t2 * k)) * 1j

        for r in range(1, 4):
            start = 10 * r
            for j in range(start, start + 4):
                if j <= 35:
                    cf[j - 1] = (t1 + t2)**r * np.sin(j) + (np.real(t1) - np.imag(t2))**2 * 1j

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_256(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        for j in range(1, 36):
            mag_part = np.log(np.abs(t1 + j) + 1) * (np.abs(t2)**(j % 5 + 1))
            angle_part = np.angle(t1) * np.sin(j) - np.angle(t2) * np.cos(j)
            cf[j - 1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))

        for k in range(1, 36):
            if k % 4 == 0:
                cf[k - 1] += np.conj(t1)**k * np.sin(t2 * k)
            elif k % 3 == 0:
                cf[k - 1] *= (np.real(t1) + np.imag(t2) * np.log(k + 1))
            else:
                cf[k - 1] += np.abs(t1) * np.abs(t2) / (k + 1)

        for r in range(1, 8):
            idx = r * 5
            if idx <= 35:
                cf[idx - 1] += 100j * t2**r - 50 * t1**r

        cf[9] = np.sum(np.abs(cf[0:9])) * np.sin(np.real(t1)) - np.cos(np.imag(t2))
        cf[19] = np.prod(np.abs(cf[14:19] + 1)) / (1 + np.abs(t1 * t2))
        cf[29] = np.conj(t1) + np.sin(t2) * np.log(np.abs(t1) + 1)
        cf[34] = np.real(t1)**2 - np.imag(t2)**2 + 1j * (np.real(t2) * np.imag(t1))

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_257(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = j % 5 + 1
            r = (j**2 + np.sin(np.real(t1) * j) - np.cos(np.imag(t2) * k)) / (np.log(np.abs(t1) + 1) + 1)
            angle = np.angle(t1)**k + np.angle(t2)**(j % 3)
            cf[j - 1] = r * (np.real(t1)**k + np.imag(t2)**j) * np.exp(1j * angle)

        for k in range(1, 6):
            r = np.prod(np.real(t1) + k) - np.sum(np.imag(t2) * k)
            angle = np.angle(t1 + k) - np.angle(t2 + k)
            index = 5 * k
            if index <= 35:
                cf[index - 1] = r * np.exp(1j * angle) * (np.sin(t1 * k) + np.cos(t2 * k))

        cf[6] = np.conj(t1) * t2**2 + np.sin(t1 + t2)
        cf[13] = np.log(np.abs(t1) + 1) * np.cos(t2) - 1j * np.sin(t1 * t2)
        cf[20] = np.abs(t1)**3 - np.abs(t2)**2 + 1j * np.angle(t1 * t2)
        cf[27] = np.real(t1**2) + np.imag(t2**3) - 2j * np.real(t1 * t2)
        cf[34] = np.prod([np.real(t1), np.real(t2)]) + np.sum([np.imag(t1), np.imag(t2)]) * 1j

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_258(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            angle = np.sin(j * np.angle(t1) + np.cos(j * np.angle(t2))) + np.real(t1) * np.imag(t2)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) + np.real(t1)**((j % 4) + 1) - np.imag(t2)**((j % 3) + 1) + np.prod([np.real(t1), np.imag(t2), j])
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))

        cf[2] = np.conj(t1) * t2**2 + np.sin(t1 * t2) * np.cos(t1 - t2)
        cf[7] = np.real(t1**2 + t2**2) + 1j * np.imag(t1 * t2)
        cf[14] = np.log(np.abs(t1 + t2) + 1) + 1j * np.angle(t1 - t2)
        cf[21] = np.sin(t1)**3 - np.cos(t2)**3 + 1j * (np.sin(t1) * np.cos(t2))
        cf[28] = np.real(t1 * t2) + np.imag(t1 + t2) * 1j
        cf[34] = np.prod([np.abs(t1), np.abs(t2), j]) + 1j * np.sum([np.real(t1), np.imag(t2)])

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_259(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        # Initialize coefficients with base patterns
        for j in range(1, 36):
            cf[j - 1] = (np.real(t1)**j * np.imag(t2)**(35 - j)) + np.conj(t1) * np.sin(j * np.angle(t2)) + \
                         np.log(np.abs(t1) + np.abs(t2) + 1) * np.cos(j * np.angle(t1 + t2))

        # Introduce variations using loops
        for k in range(1, 6):
            r = k + 5
            cf[r - 1] += np.abs(t1)**k * np.abs(t2)**(5 - k) * np.exp(1j * (np.angle(t1) - np.angle(t2)))

        for m in range(6, 11):
            cf[m - 1] += np.sin(np.real(t1) * m) + np.cos(np.imag(t2) * m)

        # Assign specific intricate coefficients
        cf[11] = np.real(t1 * t2) + 1j * np.imag(t1 / t2)
        cf[19] = np.log(np.abs(t1 + t2)) + 1j * np.angle(t1 - t2)
        cf[24] = np.conj(t1)**2 - np.conj(t2)**3 + np.sin(t1 * t2)
        cf[29] = np.abs(t1)**3 * np.abs(t2)**2 + np.cos(np.angle(t1) * np.angle(t2))
        cf[34] = np.prod([np.abs(t1), np.abs(t2)]) + np.sum([np.real(t1), np.imag(t2)]) * 1j

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_260(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, len(cf) + 1):
            k = j % 7 + 1
            r = j // 7 + 1
            cf[j - 1] = (np.real(t1)**k - np.imag(t2)**r) * np.cos(np.angle(t1) * j) + np.sin(np.angle(t2) * r) / (np.abs(t1) + np.abs(t2) + j)

        cf[3] = np.conj(t1) * t2**2 + np.log(np.abs(t1) + 1) * np.sin(t2)
        cf[7] = np.real(t1 * t2) + np.imag(t1)**2 - np.cos(t2)
        cf[12] = np.abs(t1 + t2)**2 - np.real(t1)**3 + np.imag(t2)
        cf[16] = np.sin(t1) * np.cos(t2) + np.real(t2)**2 - np.imag(t1)**2
        cf[21] = np.log(np.abs(t1 * t2) + 1) + np.conj(t1) - np.conj(t2)
        cf[25] = np.real(t1)**2 * np.imag(t2) - np.real(t2) * np.imag(t1) + np.sin(np.angle(t1 + t2))
        cf[30] = (np.real(t1) + np.imag(t1)) * (np.real(t2) - np.imag(t2)) + np.cos(np.angle(t1 * t2))
        cf[33] = np.real(t1)**3 - np.imag(t1)**3 + np.real(t2)**3 - np.imag(t2)**3
        cf[34] = np.sum([np.real(t1), np.real(t2), np.imag(t1), np.imag(t2)]) + np.prod([np.abs(t1), np.abs(t2)])

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_261(t1, t2):
    try:
        # x934
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag = np.log(1 + np.abs(t1)**j + np.abs(t2)**(35 - j)) + np.sin(j * np.angle(t1) + np.angle(t2))
            angle = np.cos(j * np.angle(t1)) - np.sin((35 - j) * np.angle(t2))
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_262(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        # Assign fixed coefficients for specific indices
        cf[[2, 7, 15, 23, 29]] = [2 + 1j, -3 + 2j, 4 - 1.5j, -2.2 + 0.8j, 0.6 - 0.4j]
        
        # Loop to assign intricate coefficients
        for j in range(1, 36):
            if j not in [3, 8, 16, 24, 30]:
                k = (j * 3) % 7 + 1
                r = (j + 4) % 5 + 1
                mag = np.log(np.abs(t1)**k + np.abs(t2)**r + j)
                ang = np.angle(t1) * k - np.angle(t2) * r + np.sin(j) * np.pi / 6
                cf[j - 1] = mag * (np.cos(ang) + np.sin(ang) * 1j)

        # Add additional intricate coefficients
        cf[11] = np.conj(t1)**2 * t2 - t1 * np.conj(t2)
        cf[18] = np.sin(t1 * t2) + np.cos(t1 + t2) * 1j
        cf[26] = np.log(np.abs(t1 + t2) + 1) + np.angle(t1 - t2) * 1j
        cf[33] = (np.real(t1) + np.imag(t2)) * np.cos(np.angle(t1)) + (np.imag(t1) - np.real(t2)) * np.sin(np.angle(t2)) * 1j
        
        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_263(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 4 == 0:
                cf[j - 1] = (np.real(t1) * j + np.imag(t2) * (35 - j)) + (np.abs(t1)**0.5 * np.angle(t2)) * 1j
            elif j % 3 == 0:
                cf[j - 1] = np.sin(t1 * j) + np.cos(t2 + j) + np.log(np.abs(t1 * t2) + 1)
            elif j % 5 == 0:
                cf[j - 1] = (np.real(t2)**j - np.imag(t1)**(j % 3)) + np.conj(t1) * np.imag(t2) * 1j
            else:
                cf[j - 1] = np.real(t1)**2 + np.imag(t2)**2 + np.sin(t1 + t2) * np.cos(t1 - t2) * 1j

        for k in range(1, 8):
            index = k * 5
            if index <= 35:
                cf[index - 1] += (t1**k - t2**k) * (k % 2) + np.log(np.abs(t1 + t2) + 1) * 1j

        for r in range(1, 6):
            idx = 7 + r * 6
            if idx <= 35:
                cf[idx - 1] += np.prod(cf[0:r]) * np.sin(t1 * r) + np.cos(t2 * r) * 1j

        cf[9] = 100j * t2**3 + 100j * t2**2 - 100 * t2 - 100
        cf[14] = 100j * t1**3 - 100j * t1**2 + 100 * t2 - 100
        cf[24] = np.real(t1 * t2) + np.imag(t1 + t2) * 1j

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_264(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for k in range(1, 36):
            r = np.log(np.abs(t1) + 1) * np.sin(np.angle(t2) * k) + np.cos(np.angle(t1) * k)
            theta = np.angle(t1)**2 / (k + 1) + np.angle(t2) * np.log(np.abs(t2) + 1)
            magnitude = (np.real(t1) + np.imag(t2))**k / (k + 2) + (np.real(t2) - np.imag(t1))**(k % 5 + 1)
            cf[k - 1] = magnitude * (np.cos(theta) + np.sin(theta) * 1j)

        for j in range(5, 36, 5):
            cf[j - 1] = np.conj(cf[j - 1]) * t1**2 - t2**3

        for r in range(3, 36, 3):
            cf[r - 1] = np.real(t1) * cf[r - 1] + np.imag(t2) * cf[r - 1]**2

        cf[0] = 1 + t1 - t2
        cf[34] = np.sin(t1 * t2) + np.cos(t1 / (np.abs(t2) + 1))

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_265(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            angle = np.angle(t1) * j + np.angle(t2) * (35 - j)
            magnitude = np.abs(t1)**((j % 5) + 1) + np.abs(t2)**((j % 7) + 1) + np.log(np.abs(t1 * t2) + 1)
            phase = np.sin(j * np.real(t1)) + np.cos(j * np.imag(t2)) + np.angle(t1 + t2)
            cf[j - 1] = magnitude * (np.cos(phase) + 1j * np.sin(phase))

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_266(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for k in range(1, 36):
            mag = np.sin(np.abs(t1) * (k**2)) + np.cos(np.abs(t2) / k) + np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1)
            ang = np.angle(t1) * k + np.angle(t2) * (35 - k) + np.sin(k) * np.cos(k)
            cf[k - 1] = mag * (np.cos(ang) + 1j * np.sin(ang))

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_267(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = (j * 3 + 7) % 35 + 1
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude = np.log(np.abs(t1) + 1) * np.real(t2)**0.5 + np.imag(t1)**2 / (j + 1)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j) + np.conj(t1) * t2**j

        cf[4] = np.real(t1) + np.imag(t2) * 1j
        cf[11] = np.abs(t1)**2 - np.abs(t2)**2 + (np.real(t1) * np.imag(t2)) * 1j
        cf[19] = np.sin(t1) + np.cos(t2) * 1j

        for r in range(25, 36):
            cf[r - 1] += np.prod([t1, t2])**r / (r + 1)

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_268(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = j + 2
            r = (j % 5) + 1
            cf[j - 1] = (np.real(t1)**k - np.imag(t2)**r) * np.sin(np.abs(t1) * j) + (np.angle(t2) + j) * np.cos(np.log(np.abs(t1) + 1))
        cf[3] = np.conj(t1) * t2**2 - np.abs(t2) * np.cos(t1)
        cf[6] = np.sin(t1 * t2) + np.cos(t1 + t2) * t1
        cf[9] = np.log(np.abs(t1) + 1) + np.real(t2)**3 - np.imag(t1) * np.imag(t2)
        cf[12] = (t1**2 + t2**2) * np.sin(t1) - np.cos(t2)
        cf[15] = np.real(t1) * np.real(t2) + np.imag(t1) * np.imag(t2) + np.angle(t1 * t2)
        cf[18] = np.abs(t1 + t2) * np.sin(np.angle(t1)) - np.cos(np.abs(t2))
        cf[21] = np.conjugate(t1**3)+t2**3 - np.log(np.abs(t1*t2)+1)
        cf[24] = np.sin(t1**2) + np.cos(t2**2) - np.real(t1 * t2)
        cf[27] = np.imag(t1**2) - np.real(t2**2) - np.real(t1 * t2)
        cf[30] = np.abs(t1)**2 * np.cos(t2) - np.sin(np.abs(t2))
        cf[33] = np.real(t1**3) - np.imag(t2**3) + np.log(np.abs(t1 + t2) + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_269(t1, t2):
    try:
        # x934
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 3 == 1:
                cf[j - 1] = (np.real(t1)**j + np.imag(t2)**(j % 5 + 1)) * np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2))
            elif j % 3 == 2:
                cf[j - 1] = (np.abs(t1) * np.abs(t2))**((j + 1) / 7) + np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1)
            else:
                cf[j - 1] = np.conj(t1) * t2**(j % 4) - np.conj(t2) * t1**(j % 3)

        # Override specific coefficients with intricate patterns
        cf[3] = np.sum([np.real(t1), np.imag(t2)]) + np.prod([np.abs(t1), np.abs(t2)])
        cf[9] = np.sin(t1 * t2) + np.cos(t1 - t2) + np.log(np.abs(t1 + t2) + 1)
        cf[15] = (np.real(t1)**2 - np.imag(t1)**2) + (np.real(t2)**2 - np.imag(t2)**2)
        cf[21] = np.abs(t1 * t2) * np.angle(t1 + t2) + np.conj(t1 - t2)
        cf[27] = np.sin(np.real(t1) * np.imag(t2)) + np.cos(np.imag(t1) * np.real(t2))
        cf[33] = np.log(np.abs(t1)**3 + np.abs(t2)**3 + 1) + np.real(t1 * np.conj(t2))

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_270(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            phase1 = np.angle(t1) * j + np.sin(j * np.angle(t2))
            magnitude1 = np.log(np.abs(t1) + j) * np.cos(j * np.pi / 7)
            term1 = magnitude1 * np.exp(1j * phase1)

            phase2 = np.angle(t2) * (35 - j) + np.cos(j * np.angle(t1))
            magnitude2 = np.log(np.abs(t2) + (35 - j)) * np.sin(j * np.pi / 5)
            term2 = magnitude2 * np.exp(1j * phase2)

            cf[j - 1] = term1 + term2 + np.conj(t1)**(j % 5) * np.conj(t2)**(j % 3)

        for k in range(2, 35, 3):
            cf[k - 1] *= (np.sin(np.abs(t1 * k)) + np.cos(np.abs(t2 + k)))

        for r in range(1, 36, 5):
            cf[r - 1] += 1j * np.log(np.abs(t1 + r) + 1) * np.sin(np.angle(t2) * r)

        cf[0] = np.real(t1) + np.real(t2)
        cf[34] = np.imag(t1) - np.imag(t2) + np.conj(t1 * t2)

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_271(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t2)) + np.cos(np.abs(t2) * j)
            angle = np.real(t1) * j + np.imag(t2) / (j + 1)
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_272(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for k in range(1, 36):
            j = (k + 3) % 6 + 1
            r = k // 4 + 1
            mag_part = np.log(np.abs(t1) + k) * np.sin(j * np.angle(t2)) + np.cos(r * np.angle(t1))
            angle_part = np.angle(t1)**j - np.angle(t2)**r + np.sin(k) * np.cos(k)
            cf[k - 1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part)) + np.conj(t1)**j * np.conj(t2)**r

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_273(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        cf[[0, 5, 11, 17, 23, 29]] = [2, -3 + 1j, 4, -5j, 6 + 2j, -7]
        for j in range(2, 35):
            if cf[j] == 0:
                cf[j] = (np.real(t1)**j - np.imag(t2)**j) + (np.angle(t1) * j + np.abs(t2)) * 1j

        for k in range(3, 34):
            cf[k] += np.sin(t1 * k) * np.cos(t2 / k) + np.log(np.abs(t1) + 1) * np.sin(np.angle(t2)) * 1j

        cf[9] = np.conj(t1) * t2**2 + np.abs(t2) * 1j
        cf[14] = np.real(t1**3) + np.imag(t2**3) * 1j
        cf[19] = np.prod([np.real(t1), np.real(t2)]) + np.prod([np.imag(t1), np.imag(t2)]) * 1j
        cf[24] = np.sum([np.abs(t1), np.abs(t2)]) + np.angle(t1 + t2) * 1j
        cf[27] = np.sin(np.abs(t1)) + np.cos(np.abs(t2)) * 1j
        cf[31] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1) * 1j
        cf[34] = np.conj(t1 + t2)

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_274(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag = np.log(np.abs(t1) + 1)**j * np.sin(j * np.angle(t1)) + np.abs(t2)**(j % 4 + 1)
            ang = np.angle(t1) * j + np.angle(t2) * (j % 5)
            cf[j - 1] = mag * (np.cos(ang) + 1j * np.sin(ang))

        for k in range(1, 36, 7):
            cf[k - 1] += 100j * t1**k - 50 * t2**(k % 3)

        for r in range(2, 35):
            cf[r - 1] = cf[r - 1] * (1 + 0.1 * r) + np.conj(t1) * np.sin(r * np.angle(t2))

        cf[0] = 1 + np.real(t1) - np.real(t2)
        cf[34] = 2 - np.imag(t1) + np.imag(t2)

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_275(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = (j * 4) % 8 + 1
            r = j // 5 + 2
            angle = np.angle(t1) * j + np.angle(t2) * k + np.sin(j) * np.cos(k)
            mag = np.abs(t1)**j + np.abs(t2)**k + np.log(np.abs(t1 * t2) + 1) * r
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1)**r * np.conj(t2)**k

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_276(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j % 5 == 1:
                cf[j - 1] = np.real(t1)**j + np.imag(t2)**(j % 3 + 1) * np.conj(t1)
            elif j % 5 == 2:
                cf[j - 1] = np.abs(t1 + t2)**j * np.sin(np.angle(t1) * j) + 1j * np.cos(np.angle(t2) * j)
            elif j % 5 == 3:
                cf[j - 1] = np.log(np.abs(t1) + 1) * np.real(t2)**j - 1j * np.log(np.abs(t2) + 1) * np.imag(t1)**j
            elif j % 5 == 4:
                cf[j - 1] = (t1**2 + t2**3) * np.sin(j) + 1j * (t1 * t2)**2 * np.cos(j)
            else:
                cf[j - 1] = np.prod([np.real(t1), np.imag(t2), j]) + 1j * np.sum([np.abs(t1), np.abs(t2), j])

        cf[4] = 100 * t1**4 - 50j * t2**2 + 25
        cf[9] = 200j * np.sin(t1) + 150 * np.cos(t2)
        cf[14] = 300 * np.log(np.abs(t1) + 1) + 100j * np.log(np.abs(t2) + 1)
        cf[19] = np.conj(t1) * t2**3 - t1**2 * np.conj(t2)
        cf[24] = np.abs(t1)**3 + np.abs(t2)**2 * 1j
        cf[29] = np.sin(t1 * t2) + np.cos(t1 + t2) * 1j
        cf[34] = np.log(np.abs(t1 * t2) + 1) + 1j * np.angle(t1 + t2)

        return cf.astype(np.complex128).astype(np.complex128)

    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_277(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(35):  # Python uses 0-based indexing
            if j < 5:
                k = j + 1
                cf[j] = (np.real(t1)**k + np.imag(t2)**k) * np.exp(1j * np.angle(t1 + t2))
            elif j < 10:
                k = j - 4
                cf[j] = (np.abs(t1)**k * np.abs(t2)**5) / (k + 1) + 1j * np.sin(k * np.angle(t1))
            elif j < 15:
                k = j - 9
                cf[j] = np.real(t1 * t2) + 1j * np.imag(t1**k + t2**k)
            elif j < 20:
                k = j - 14
                cf[j] = np.log(np.abs(t1) + 1) * np.cos(k * np.angle(t2)) + 1j * np.log(np.abs(t2) + 1) * np.sin(k * np.angle(t1))
            elif j < 25:
                k = j - 19
                cf[j] = (t1 + np.conj(t2))**k + (np.conj(t1) - t2)**k
            elif j < 30:
                k = j - 24
                cf[j] = np.real(t1)**k * np.imag(t2)**k + 1j * (np.abs(t1 + t2)**k)
            else:
                k = j - 29
                cf[j] = (np.real(t1) * np.imag(t2))**k + np.conj(t1 * t2)**k
        
        cf[11] = 100 * t1**3 - 50j * t2**2 + 25 * t1 * t2
        cf[17] = 200j * np.sin(t1) + 150 * np.cos(t2)
        cf[26] = 300 * np.log(np.abs(t1) + 1) + 100j * np.abs(t2)**2
        cf[33] = 400 * np.real(t1 * t2) - 200j * np.imag(t1 + t2)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_278(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        
        fixed_indices = [0, 3, 9, 15, 21, 29]  # Adjusted for 0-based indexing
        cf[fixed_indices] = [2, -3, 5 + 2j, -4 + 1j, 3.5, -2.2]
        
        for j in range(1, 35):
            if j not in fixed_indices:
                angle = np.angle(t1)**0.5 * (j+1) + np.angle(t2)**0.3 * (35 - (j+1))
                magnitude = np.abs(t1)**((j+1) / 3) + np.abs(t2)**(35 - (j+1))/2
                cf[j] = magnitude * (np.cos(angle) + np.sin(angle)*1j)
        
        cf[6] = (100 * t1**2 - 50 * np.conj(t2)) + (25 * np.sin(t1) + 75 * np.cos(t2))*1j
        cf[13] = (200 * t2**3 + 100 * np.real(t1)) + (50 * np.imag(t2) - 30 * np.log(np.abs(t1)+1))*1j
        cf[20] = (np.abs(t1) + np.abs(t2)) + (np.real(t1) * np.real(t2))*1j
        cf[27] = (np.log(np.abs(t1) + 1) * np.real(t1)) - (np.real(t2)**2) + (np.imag(t1) * np.imag(t2))*1j
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_279(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(35):
            mag = np.log(np.abs(t1)**(j+1) + np.abs(t2)**(35-(j+1)) + 1) * ((j+1) % 7 + 1) * (1 + np.sin(j+1))
            ang = np.angle(t1) * (j+1)**0.5 - np.angle(t2) * (35 - (j+1))**0.3
            cf[j] = mag * (np.cos(ang) + 1j * np.sin(ang))
        
        for k in range(35):
            if (k+1) % 5 == 0:
                cf[k] = cf[k] * np.conj(t1) + np.real(t2)**2
            elif (k+1) % 3 == 0:
                cf[k] = cf[k] + np.imag(t1) * np.imag(t2)
            else:
                cf[k] = cf[k] * np.real(t1 + t2) - np.imag(t1 - t2)
        
        indices = [2, 7, 14, 22, 28, 34]
        cf[indices] = cf[indices] + 100 * t1**2 - 50 * t2
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_280(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(35):
            if (j+1) % 6 == 1:
                cf[j] = (t1**(j+1) + np.conj(t2)**(j+1)) * np.log(np.abs(t1) + 1)
            elif (j+1) % 6 == 2:
                cf[j] = (np.sin(t1 * (j+1)) + np.cos(t2 * (j+1))) * ((j+1)**2 + np.real(t1))
            elif (j+1) % 6 == 3:
                cf[j] = (np.real(t1) * np.imag(t2))**(j+1) + np.conj(t1 * t2)
            elif (j+1) % 6 == 4:
                cf[j] = np.log(np.abs(t1 + t2) + 1) * ((j+1)**1.5) * np.angle(t1 + t2)
            elif (j+1) % 6 == 5:
                cf[j] = (np.real(t1)**2 - np.imag(t2)**2) * (j+1) + 1j * (np.imag(t1) + np.real(t2))
            else:
                cf[j] = (np.abs(t1) + np.abs(t2)) * (j+1)**3 * np.sin(np.angle(t1 * t2))
        
        indices = [2, 7, 14, 21, 28, 33]
        cf[indices] = [2 + 3j, -1 + 4j, 0.5 - 2j, 3 + 0j, -2.5j, 1 + 1j]
        cf[9] = 100j * (t1**3) - 50 * t2**2 + 25 * np.conj(t1)
        cf[19] = 75 * t2**3 + 50j * np.conj(t2) - 25 * t1
        cf[24] = 60j * np.sin(t1) * np.cos(t2) + 40 * np.log(np.abs(t1 * t2) + 1)
        cf[34] = 150 * np.real(t1 + t2) - 100j * np.imag(t1 - t2)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_281(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        cf[[0, 4, 9, 14, 19, 24, 29, 34]] = np.array([
            np.real(t1) + np.imag(t2) * 1j,
            np.abs(t1)**2 - np.abs(t2)**2 * 1j,
            np.sin(np.angle(t1)) + np.cos(np.angle(t2)) * 1j,
            np.log(np.abs(t1) + 1) - np.log(np.abs(t2) + 1) * 1j,
            np.conj(t1) + np.conj(t2) * 1j,
            np.real(t1)**3 - np.imag(t2)**3 * 1j,
            np.abs(t1)**4 + np.abs(t2)**4 * 1j,
            np.sin(np.angle(t1) * 2) - np.cos(np.angle(t2) * 2) * 1j
        ])
        for j in [2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 16, 17, 18, 19, 21, 22, 23, 24, 26, 27, 28, 29, 31, 32, 33, 34]:
            k = j * 3
            r = j % 5
            if r == 0:
                cf[j] = (np.real(t1) + np.imag(t2)) * np.sin(k) + (np.real(t2) - np.imag(t1)) * np.cos(k) * 1j
            elif r == 1:
                cf[j] = np.abs(t1 + t2)**k * np.exp(1j * np.angle(t1 - t2))
            elif r == 2:
                cf[j] = np.log(np.abs(t1)**k + 1) + np.log(np.abs(t2)**k + 1) * 1j
            elif r == 3:
                cf[j] = np.conj(t1)**k - np.conj(t2)**k * 1j
            else:
                cf[j] = np.sum(np.real(t1), np.imag(t2)) * np.prod(np.abs(t1), np.abs(t2)) + 1j * np.sum(np.imag(t1), np.real(t2))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_282(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        cf[[0, 6, 13, 20, 27, 34]] = np.array([2.5, -4.2, 3.8, -16.5, 5.3, 0.6])
        
        for j in range(2, 35):
            if j % 4 == 0:
                k = j // 2
                cf[j] = (150j * t1**k + 75 * np.conj(t2)) * np.sin(k * np.angle(t1)) - 50 * np.log(np.abs(t2) + 1)
            elif j % 3 == 0:
                k = j % 5
                cf[j] = (200 * np.real(t1 * t2**k) + 100j * np.imag(t1 - t2)) * np.cos(k * np.angle(t2))
            else:
                r = j % 7
                cf[j] = np.conj(t1)**r * t2**j + np.abs(t1**j) * np.abs(t2**r)
        
        cf[9] = 180j * t1**3 - 120 * t2**2 + 90 * np.sin(t1) * np.cos(t2)
        cf[19] = 220j * t2**4 + 130 * np.real(t1**3) - 100 * np.imag(t2)
        cf[29] = 260j * t1**2 * t2 + 160 * np.log(np.abs(t1 * t2) + 1) - 110 * np.conj(t1)
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_283(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            angle_part = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude_part = np.abs(t1)**((j % 5) + 1) + np.abs(t2)**((j % 7) + 1)
            cf[j - 1] = magnitude_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
            if j % 4 == 0:
                cf[j - 1] += np.conj(t1) * t2**2 - np.log(np.abs(t1) + 1)
            if j % 6 == 0:
                cf[j - 1] *= np.sin(t1 * j) + np.cos(t2 / (j + 1))
        specific_indices = [3, 8, 15, 22, 29, 35]
        for k in specific_indices:
            cf[k - 1] += (np.real(t1) + np.imag(t2)) * t1**k - (np.real(t2) - np.imag(t1)) * t2**k
        cf[[4, 11, 18, 25, 32]] = np.array([5, -10, 15, -20, 25]) + 1j * np.array([-5, 10, -15, 20, -25])
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_284(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            phase = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude = np.abs(t1)**((j % 5) + 1) + np.abs(t2)**(j // 7 + 1)
            perturb = np.log(np.abs(t1 + t2) + 1) * np.cos(j * np.pi / 3) + np.sin(j * np.pi / 4)
            cf[j - 1] = magnitude * np.exp(1j * phase) + perturb
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_285(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            real_seq = np.linspace(np.real(t1), np.real(t2), j)
            imag_seq = np.linspace(np.imag(t1), np.imag(t2), j)
            mag_component = np.sum(np.log(np.abs(real_seq) + 1) * np.sin(real_seq * j)) + np.prod(imag_seq + 1)
            angle_component = np.sum(np.cos(imag_seq * j)) - np.sum(np.sin(real_seq / (j + 1)))
            cf[j - 1] = mag_component * (np.cos(angle_component) + 1j * np.sin(angle_component))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_286(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            a = np.real(t1) * np.sin(k * np.real(t2)) + np.imag(t1) * np.cos(k * np.imag(t2))
            b = np.log(np.abs(t1) + 1) * np.sin(k * np.angle(t2) / (k + 1))
            c = np.abs(t2)**k * np.cos(k * np.real(t1))
            d = np.sin(k * np.imag(t1)) + np.cos(k * np.real(t2))
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k)
            magnitude = a + b + c + d
            cf[k - 1] = magnitude * np.exp(1j * angle) + np.conj(t1) * np.conj(t2)**k
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_287(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        
        for j in range(1, n + 1):
            r = rec_seq[j - 1]
            m = imc_seq[j - 1]
            
            mag_part = np.log(np.abs(r * m) + 1) * (j**2 + np.sin(j) * np.cos(j))
            angle_part = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 4) + np.sin(m * np.pi / 5)
            
            coeff = mag_part * np.exp(1j * angle_part) + np.conj(t1) * np.conj(t2) / (j + 1)
            cf[j - 1] = coeff
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_288(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag_sum = 0
            angle_sum = 0
            for k in range(1, j + 1):
                term_mag = np.log(np.abs(rec[k - 1] * imc[j - 1]) + 1) * np.sin(k * np.pi / n)
                term_angle = np.angle(rec[k - 1] + 1j * imc[j - 1]) + np.cos(k * np.pi / (n + 1))
                mag_sum += term_mag
                angle_sum += term_angle
            magnitude = mag_sum * np.prod(np.repeat(np.abs(t1) + k, j % 3 + 1))
            angle = angle_sum / (j + 1) + np.sin(j * np.pi / (n + 2)) * np.cos(j * np.pi / (n + 3))
            variation = np.sin(j) if j % 2 == 0 else np.cos(j)
            cf[j - 1] = magnitude * np.exp(1j * angle) + variation * np.conj(t2)**j
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_289(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag_factor = np.log(np.abs(rec_seq[j - 1] + imc_seq[j - 1] * 1j) + 1) * (1 + np.sin(j * np.pi / 4))
            angle_factor = np.angle(rec_seq[j - 1] + 1j * imc_seq[j - 1]) + np.cos(j * np.pi / 3) * np.sin(j * np.pi / 5)
            cf[j - 1] = mag_factor * (np.cos(angle_factor) + 1j * np.sin(angle_factor))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_290(t1, t2):
    try:
        n = 34
        cf = np.zeros(35, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), 35)
        imc = np.linspace(np.imag(t1), np.imag(t2), 35)
        for j in range(1, 36):
            if j % 4 == 1:
                mag = np.log(np.abs(t1) + j**2) + np.sin(j * np.pi / 6) * np.cos(j * np.pi / 4)
                angle = np.angle(t1) * j + np.sin(j * np.pi / 5) - np.cos(j * np.pi / 3)
            elif j % 4 == 2:
                mag = np.log(np.abs(t2) + j) * np.prod(np.arange(1, (j % 5) + 2))
                angle = np.angle(t2) / (j + 1) + np.sin(j * np.pi / 7)
            elif j % 4 == 3:
                mag = np.real(t1) * j - np.imag(t2) + np.log(np.abs(t1 + t2) + 1)
                angle = np.angle(t1 * t2) + np.cos(j * np.pi / 2)
            else:
                mag = np.abs(np.real(t1 - t2)) * j**1.5 + np.sin(j * np.pi / 3)
                angle = np.angle(t1 - t2) + np.sin(j * np.pi / 4)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_291(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag_part1 = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * np.pi / 7))
            mag_part2 = np.prod(np.arange(1, j + 1))**0.5 / (1 + np.abs(np.real(t1 - t2)) / (j + 1))
            magnitude = mag_part1 * mag_part2 * (1 + np.cos(j * np.pi / 5))
            
            angle_part1 = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 4)
            angle_part2 = np.sum(np.sin(np.arange(1, j + 1) * np.pi / 6)) - np.sum(np.cos(np.arange(1, j + 1) * np.pi / 8))
            angle = angle_part1 + angle_part2
            
            real_component = np.real(t1) * np.cos(j) - np.imag(t2) * np.sin(j)
            imag_component = np.real(t2) * np.sin(j) + np.imag(t1) * np.cos(j)
            perturbation = np.sin(real_component) + np.cos(imag_component)
            
            cf[j - 1] = magnitude * np.exp(1j * angle) * perturbation
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_292(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            k = (j**2 + 3 * j + 1) % n + 1
            r = np.sin(j * np.real(t1)) * np.cos(k * np.imag(t2))
            angle = np.angle(t1) * j - np.angle(t2) * k + np.log(j + 1)
            magnitude = np.abs(t1)**0.5 * np.abs(t2)**0.3 * np.abs(r) + j
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_293(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            k = (j * 2 + 5) % 12
            r = j // 6 + 1
            term_re = np.real(t1) * np.sin(j) + np.real(t2) * np.cos(k)
            term_im = np.imag(t1) * np.cos(j / 4) - np.imag(t2) * np.sin(k / 3)
            magnitude = (np.abs(term_re) + np.abs(term_im)) * np.log(1 + j) * (j**0.4)
            angle = np.angle(t1) * np.sin(j / 2) + np.angle(t2) * np.cos(k / 4) + np.log(j + 2)
            cf[j - 1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_294(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            k = (j % 7) + 1
            r = rec[j - 1] * np.cos(j) - imc[j - 1] * np.sin(j)
            i_part = rec[j - 1] * np.sin(j) + imc[j - 1] * np.cos(j)
            mag = np.log(np.abs(r + 1) + np.abs(i_part + 1)) * (1 + np.sin(j * np.pi / k)) * (1 + np.cos(j * np.pi / (k + 1)))
            angle = np.angle(t1) + np.angle(t2) + np.sin(j * np.pi / k) + np.cos(j * np.pi / (k + 2))
            cf[j - 1] = mag * np.exp(1j * angle) + np.conj(t1 * t2) / (j + 2)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_295(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            r = rec[j - 1] + imc[j - 1]
            magnitude = np.log(np.abs(r + 1j) + 1) * (j**(np.sin(j) + 1))
            angle = np.angle(r + 1j) + np.sin(j * np.pi / 4) * np.cos(j * np.pi / 3)
            cf[j - 1] = magnitude * np.exp(1j * angle) + np.conj(magnitude * np.exp(1j * (angle / 2))) * np.cos(j * np.pi / 5)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_296(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), 35)
        imc = np.linspace(np.imag(t1), np.imag(t2), 35)
        for k in range(1, 36):
            j = (k + 7) % 12 + 1
            term1 = rec[k - 1] * np.cos(imc[j - 1] * np.pi / 5)
            term2 = imc[k - 1] * np.sin(rec[j - 1] * np.pi / 4)
            conj_part = np.conj(t1) * np.conj(t2)
            angle = np.angle(term1 + term2 + np.angle(conj_part))
            magnitude = np.log(np.abs(term1 + term2) + 1) * (np.abs(t1)**((k % 4) + 1)) * (np.abs(t2)**((j % 3) + 1))
            cf[k - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_297(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t2)) + np.cos(j**2 * np.real(t1))
            angle_part = np.angle(t1) * np.log(j + 1) + np.angle(t2) * np.sqrt(j)
            cf[j - 1] = mag_part * np.exp(1j * angle_part) + np.conj(t1)**j / (1 + np.abs(t2 + j))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_298(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(rec[j - 1]) + 1) * np.sin(j * np.pi / 7) + np.cos(j * np.pi / 5)
            angle_part = np.angle(t1) * j**0.5 - np.angle(t2) / (j + 2)
            fluctuation = np.abs(t1) * np.abs(t2) if j % 3 == 0 else np.abs(t1 + t2) / (j + 1)
            cf[j - 1] = (mag_part + fluctuation) * np.exp(1j * angle_part) + np.conj(t1 * t2)**j
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_299(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag = np.abs(t1)**j + np.abs(t2)**(n - j + 1) + np.sum(np.sin(j * np.pi / (np.arange(1, 6) + 1)))
            ang = np.angle(t1) * np.log(j + 1) + np.angle(t2) * np.arctan(j) + np.sum(np.cos((np.arange(1, 4)) * np.pi / j))
            cf[j - 1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_300(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r1 = np.real(t1)
            r2 = np.real(t2)
            i1 = np.imag(t1)
            i2 = np.imag(t2)
            term_mag = np.log(np.abs(t1) + j) * np.abs(r1 * j - i2 / (j + 1)) + np.prod(np.array([r1, i2, j]))
            term_angle = np.angle(t1) * j - np.angle(t2) * (n - j) + np.sin(j * r2) * np.cos(j * i1)
            cf[j - 1] = term_mag * (np.cos(term_angle) + 1j * np.sin(term_angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_301(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part1 = np.log(np.abs(np.real(t1) * j) + 1) * np.sin(j * np.pi / 4)
            mag_part2 = np.cos(j * np.pi / 3) * np.abs(t2)**0.5
            magnitude = mag_part1 + mag_part2 + np.sum(np.arange(1, j + 1)) / np.prod(np.arange(1, min(j, 6) + 1))
            
            angle_part1 = np.angle(t1) * np.sin(j / 2)
            angle_part2 = np.angle(t2) * np.cos(j / 3)
            phase = angle_part1 + angle_part2 + np.sin(j) * np.cos(j / 2)
            
            cf[j - 1] = magnitude * np.exp(1j * phase) + np.conj(t1) * np.sin(j * np.pi / 6) - np.conj(t2) * np.cos(j * np.pi / 5)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_302(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag = np.log(np.abs(t1) + j) * (1 + np.sin(rec[j - 1] * imc[j - 1])) + np.prod(np.array([np.real(t1), np.imag(t2)])) / (j + 1)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j) + np.sin(rec[j - 1] / (imc[j - 1] + 1))
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1) * np.cos(j / 3)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_303(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            r1 = np.real(t1)
            i1 = np.imag(t1)
            r2 = np.real(t2)
            i2 = np.imag(t2)
            term1 = np.log(np.abs(r1 * j) + 1) * np.sin(j * np.pi / 3)
            term2 = np.log(np.abs(i2 + j) + 1) * np.cos(j * np.pi / 4)
            term3 = np.real(np.conj(t1) * t2) / (j + 1)
            mag = np.abs(term1 + term2 + term3)
            angle = np.angle(t1) * np.cos(j * np.pi / 6) + np.angle(t2) * np.sin(j * np.pi / 8) + np.log(np.abs(r1 + i1 * i2) + 1) / (j + 2)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_304(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for k in range(1, n + 1):
            r = rec[k - 1]
            im = imc[k - 1]
            conj_t1 = np.conj(t1)
            conj_t2 = np.conj(t2)
            mag_component = np.log(np.abs(t1) + np.abs(t2) + 1) * np.sin(k * np.angle(t1 * conj_t2) + np.cos(k * np.pi / 6))
            angle_component = np.angle(t1)**k - np.angle(t2)**(n - k) + np.sin(k * np.pi / 5)
            magnitude = np.abs(mag_component + r * np.prod(imc[:k]) / (k + 1))
            cf[k - 1] = magnitude * (np.cos(angle_component) + 1j * np.sin(angle_component))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_305(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        real_seq = np.linspace(np.real(t1), np.real(t2), n)
        imag_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for k in range(1, n + 1):
            r = real_seq[k - 1]
            im = imag_seq[k - 1]
            mag_pattern = np.log(np.abs(t1 * t2) + k**2) * (1 + np.sin(k) * np.cos(k / 2))
            angle_pattern = np.angle(t1) * np.sin(k / 3) + np.angle(t2) * np.cos(k / 4) + np.sin(k * np.pi / 5)
            cf[k - 1] = mag_pattern * np.exp(1j * angle_pattern) + np.conj(mag_pattern * np.exp(1j * angle_pattern / 2))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_306(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = np.real(t1)
        rec2 = np.real(t2)
        imc1 = np.imag(t1)
        imc2 = np.imag(t2)
        for j in range(1, n + 1):
            k = j % 5 + 1
            r = np.log(np.abs(rec1 * rec2) + 1) + np.log(np.abs(t1 * t2) + 1)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude = np.abs(t1)**k + np.abs(t2)**(n - k) + r * np.sin(j * np.pi / 7)
            phase = angle + np.cos(j * np.pi / 5)
            cf[j - 1] = magnitude * (np.cos(phase) + 1j * np.sin(phase))
            if j % 7 == 0:
                cf[j - 1] = cf[j - 1] * np.conj(t1) + np.conj(t2)
            if j % 3 == 0:
                cf[j - 1] += np.sin(t1 * j) * np.cos(t2 / j)
            if j % 4 == 0:
                cf[j - 1] += np.exp(1j * (rec1 * j - imc2))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_307(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            phase = np.sin(j * np.pi / 4) + np.cos(j * np.pi / 3) + np.angle(t1) * j / 10
            magnitude = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.pi / 6)) + np.prod(np.arange(1, j + 1))**0.5 * np.cos(j * np.pi / 8)
            cf[j - 1] = magnitude * np.exp(1j * phase) + np.conj(t2) * (j % 5)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_308(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            magnitude = (np.abs(t1) * np.log(np.abs(rec[j - 1]) + 1) * j) if j % 2 == 0 else (np.abs(t2) * np.sin(rec[j - 1]) + np.abs(t1 + t2) / (j + 1))
            angle = (np.angle(t1) + np.sin(imc[j - 1] * np.pi / j)) if j <= n / 2 else (np.angle(t2) + np.cos(rec[j - 1] * np.pi / j))
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, n + 1):
            cf[k - 1] += np.conj(cf[k - 1]) * np.exp(1j * (np.sin(k) + np.cos(k)))
        for r in range(1, n + 1):
            cf[r - 1] *= (1 + np.log(np.abs(cf[r - 1]) + 1)) / (1 + r / n)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_309(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag = np.log(np.abs(rec_seq[j - 1] + imc_seq[j - 1] * 1j) + 1) * (1 + np.sin(j * np.pi / 4)) * (1 + np.cos(j * np.pi / 5))
            angle = np.sin(j * rec_seq[j - 1]) + np.cos(j * imc_seq[j - 1]) + np.angle(t1 * t2) / (j + 1)
            cf[j - 1] = mag * np.exp(1j * angle) + np.conj(t2) * np.sin(j / n * np.pi)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_310(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(t1) + 1) * np.sin(j * np.pi / 5) + np.log(np.abs(t2) + 1) * np.cos(j * np.pi / 7) + j**1.5
            ang_part = np.angle(t1) * np.sin(j * np.pi / 4) + np.angle(t2) * np.cos(j * np.pi / 6) + np.sin(j / 3)
            cf[j - 1] = mag_part * np.exp(1j * ang_part)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_311(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(rec[j - 1] * t1 + imc[j - 1] * t2) + 1) * (1 + np.sin(j) * np.cos(j))
            angle_part = np.sin(j * np.pi * imc[j - 1]) + np.cos(j * np.pi * rec[j - 1]) + np.angle(t1) - np.angle(t2)
            cf[j - 1] = mag_part * np.exp(1j * angle_part)
        
        for k in range(1, n + 1):
            r = np.log(k + 1)
            cf[k - 1] *= (1 + np.sin(r) + 1j * np.cos(r))
        
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_312(t1, t2):
    try:
        n = 35
        cf = np.zeros(35, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), 35)
        imc = np.linspace(np.imag(t1), np.imag(t2), 35)
        for j in range(1, 36):
            magnitude = np.log(np.abs(rec[j - 1] * imc[j - 1]) + 1) * np.sin(j * np.pi / 4) + np.cos(j * np.pi / 3)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j) + np.log(np.abs(rec[j - 1] + imc[j - 1]) + 1)
            cf[j - 1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_313(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            if j % 4 == 1:
                mag = np.log(np.abs(rec[j - 1] * imc[j - 1]) + 1) * (j**1.3 + np.sqrt(j))
                ang = np.sin(j * np.pi * rec[j - 1]) + np.cos(j**2 * np.pi * imc[j - 1]) + np.angle(t1) * np.real(t2)
            elif j % 4 == 2:
                mag = np.log(np.abs(rec[j - 1] + imc[j - 1]) + 1) * (np.exp(0.05 * j) + j)
                ang = np.cos(j * np.pi * rec[j - 1]) - np.sin(j**1.5 * np.pi * imc[j - 1]) + np.angle(t2) * np.imag(t1)
            elif j % 4 == 3:
                mag = np.log(np.abs(rec[j - 1] - imc[j - 1]) + 1) * (j**2 / (1 + j))
                ang = np.sin(j**2 * np.pi * rec[j - 1]) * np.cos(j * np.pi * imc[j - 1]) + np.angle(t1) * np.angle(t2)
            else:
                mag = np.log(np.abs(rec[j - 1]**2 + imc[j - 1]**2) + 1) * np.sqrt(j) * (1 + np.log(j))
                ang = np.sin(j * np.pi * rec[j - 1] / 2) + np.cos(j**3 * np.pi * imc[j - 1] / 3)
            cf[j - 1] = mag * np.exp(1j * ang)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_314(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            j = k
            r = (j**2 + np.real(t1) * np.imag(t2)) % 7 + 1
            angle = np.angle(t1) * np.sin(j * np.pi / r) + np.angle(t2) * np.cos(j * np.pi / (r + 1))
            magnitude = np.abs(t1)**(0.5 * j) + np.abs(t2)**(0.3 * (n - j + 1))
            cf[k - 1] = magnitude * np.exp(1j * angle) + np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
            cf[k - 1] = cf[k - 1] * np.log(np.abs(cf[k - 1]) + 1) + np.prod(np.arange(1, (j % 5) + 2)) + np.sum(j, r)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_315(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = np.real(t1) * np.log(j + 1) + np.real(t2) * np.sin(j)
            q = np.imag(t1) * np.cos(j / 2) + np.imag(t2) * np.log(j + np.abs(t1 * t2))
            magnitude = np.abs(r)**(j % 5 + 1) + np.abs(t2)**(j % 3 + 2)
            angle = np.angle(q) * np.sin(j) - np.angle(t2) * np.cos(j / 3)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_316(t1, t2, err=False):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            k = j + 3
            r = np.real(t1) * np.sin(j * np.pi / 8) + np.real(t2) * np.cos(j * np.pi / 5)
            im = np.imag(t1) * np.cos(j * np.pi / 7) - np.imag(t2) * np.sin(j * np.pi / 9)
            mag = np.log(np.abs(r * im) + 1) * (1 + np.sin(k * np.pi / 4)) * np.prod(np.arange(1, j+1)) / n
            ang = np.angle(t1) * np.cos(k * np.pi / 6) + np.angle(t2) * np.sin(k * np.pi / 10)
            cf[j - 1] = mag * (np.cos(ang) + 1j * np.sin(ang))
            cf = cf / np.sum(np.abs(cf))
        return cf.astype(np.complex128).astype(np.complex128)
    except Exception as e:
        if err:
            print(f"Exception type: {type(e).__name__}")
            print(f"Exception message: {str(e)}")
            return np.zeros(35, dtype=np.complex128)
        else:
            return np.zeros(35, dtype=np.complex128)

def poly_317(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j) + np.cos(j**2)
            angle_part = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j - 1] = mag_part * np.exp(1j * angle_part) + np.conj(t1) * np.prod(np.arange(1, j + 1)) / (j + 1)
        for k in range(1, n + 1):
            cf[k - 1] *= (1 + 0.05 * np.cos(k) + 0.03j * np.sin(k))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_318(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = np.real(t1)
        imc1 = np.imag(t1)
        rec2 = np.real(t2)
        imc2 = np.imag(t2)
        for j in range(1, n + 1):
            angle = np.sin(j * rec1) + np.cos(j * imc2) + np.angle(t1) * np.angle(t2) / (j + 0.1)
            magnitude = np.abs(t1)**j * np.log(np.abs(t2) + j) * (1 + (-1)**j * 0.5)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_319(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = np.real(t1) * np.log(j + 1) + np.real(t2) * np.sin(j * np.pi / 7)
            q = np.imag(t1) * np.cos(j * np.pi / 5) - np.imag(t2) * np.log(j + 2)
            magnitude = np.log(np.abs(r + 1j) + 1) * (1 + (j % 4))
            angle = np.angle(q) * np.sin(j) + np.angle(t2) * np.cos(j / 3)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_320(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            mag_part1 = np.sin(rec[j] * np.pi / (j + 1))
            mag_part2 = np.cos(imc[j] * np.pi / (j + 2))
            mag = (mag_part1 + mag_part2) * np.log(np.abs(t1) + np.abs(t2) + j) * (1 + j / n)
            ang_part1 = np.angle(t1) * rec[j] / n
            ang_part2 = np.angle(t2) * imc[j] / n
            angle = ang_part1 - ang_part2 + np.sin(j * np.pi / 5)
            cf[j] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_321(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            mag_part = (j**1.8 + np.log(np.abs(t1) + np.abs(t2) + j)) * np.abs(np.sin(j * np.real(t1)) + np.cos(j * np.imag(t2)))
            angle_part = np.angle(t1) * np.log(j + 1) + np.angle(t2) * np.sin(j / 3)
            cf[j] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_322(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            r = rec[j]
            d = imc[j]
            mag = np.log(np.abs(r + 1j) + 1) * (1 + np.sin(j * np.pi / 5) * np.cos(j * np.pi / 3))
            angle = np.angle(d) * np.sin(j * np.pi / 4) + np.angle(t2) * np.cos(j * np.pi / 6)
            cf[j] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_323(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            mag = np.real(t1)**(j % 5 + 1) * np.log(np.abs(t2) + j) + np.imag(t1) * np.sin(j * np.pi / 7)
            angle = np.angle(t1) * np.cos(j / 3) + np.angle(t2) * np.sin(j / 4)
            cf[j] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_324(t1, t2):
    try:
        n = 35
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            r = rec[j]
            d = imc[j]
            mag = np.log(np.abs(r**2) + 1) * (1 + np.sin(2 * np.pi * r * j)) * (1 + np.cos(np.pi * 1j * j))
            ang = np.angle(r + d * 1j) + np.sin(j) * np.log(np.abs(r + 1j)) - np.cos(j) * np.angle(r - 1j)
            cf[j] = mag * (np.cos(ang) + np.sin(ang) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_325(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for k in range(n):
            j = k % 5 + 1
            r = rec[k]
            d = imc[k]
            mag = np.log(np.abs(t1) + k) * np.sin(j * np.pi / 4) + np.prod(rec[max(0, k-3):k])**(1/3)
            angle = np.angle(d) * np.sin(j * np.pi / 6) + np.angle(t2) * np.cos(j * np.pi / 8) + np.imag(t2) / (k + 1)
            cf[k] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_326(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            k = (j % 7) + 1
            r = j + k
            mag = np.log(np.abs(rec[j] * imc[j]) + 1) * np.sin(j / 3) + np.cos(j / 4) * np.abs(t1 + t2)
            angle = np.angle(t1) * np.cos(j * np.pi / 6) + np.angle(t2) * np.sin(j * np.pi / 8) + np.sin(j)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_327(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            part1 = np.sin(j * np.pi / 4) * np.real(t1)**0.5
            part2 = np.cos(j * np.pi / 3) * np.imag(t2)**0.3
            part3 = np.log(np.abs(rec_seq[j] * imc_seq[j]) + 1)
            magnitude = part1 + part2 + part3
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_328(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            k = j % 5 + 1
            r = (j**2 + np.sin(j) * np.cos(j)) / (np.log(np.abs(t1) + 1) + 1)
            if j <= n / 2:
                mag_variation = r * (1 + np.sin(j * np.pi / 7))
            else:
                mag_variation = r * (1 + np.cos(j * np.pi / 5))
            angle_variation = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j) + np.log(mag_variation + 1)
            cf[j] = mag_variation * np.exp(1j * angle_variation)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_329(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            mag_part1 = np.log(np.abs(rec[j] + imc[j]) + 1)
            mag_part2 = np.sin(j * np.pi / 4) * np.cos(j * np.pi / 3)
            magnitude = mag_part1 * (1 + mag_part2)
            angle_part1 = np.angle(np.conj(t1) * t2) + np.sin(j * np.pi / 5)
            angle_part2 = np.cos(j * np.pi / 7) * np.angle(rec[j] + 1j * imc[j])
            angle = angle_part1 + angle_part2
            cf[j] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_330(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            rec = rec_seq[j]
            imc = imc_seq[j]
            mag = np.log(np.abs(rec * j + imc * j**2) + 1) * np.sin(j * np.pi / 4) + \
                  np.cos(j * np.pi / 3) * (np.abs(rec - imc) + 1)
            ang = np.angle(t1) * np.sin(j * np.pi / 6) + np.angle(t2) * np.cos(j * np.pi / 8) + np.log(j + 1)
            cf[j] = mag * (np.cos(ang) + np.sin(ang) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_331(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            mag_part1 = np.log(np.abs(t1) + j)
            mag_part2 = np.log(np.abs(t2) + n - j)
            mag = mag_part1 * mag_part2 + np.sin(j) * np.cos(j / 2)
            angle_part1 = np.angle(t1) * np.sin(j / 3)
            angle_part2 = np.angle(t2) * np.cos(j / 4)
            angle = angle_part1 + angle_part2 + np.sin(j * np.pi / 7)
            cf[j] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_332(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            temp = 0 + 0j
            for k in range(1, 6):
                r = j * k
                temp += ((rec[j] + np.imag(t1) * k)**k * np.conj(t2)**r) / (np.log(np.abs(t1) + k) + 1)
            magnitude = np.log(np.abs(temp) + 1) * np.sin(j * np.angle(t1) + np.cos(j * np.angle(t2)))
            angle = np.angle(temp) + np.sin(j) * np.cos(k)
            cf[j] = magnitude * np.exp(1j * angle) + np.abs(t1)**j - np.abs(t2)**j
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_333(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            angle = np.sin(j * np.imag(t1)) + np.cos(j * np.real(t2)) + np.angle(t1) * np.angle(t2) / j
            magnitude = np.log(np.abs(t1) + 1) * (j**1.5) + np.exp(-j / (np.abs(t2) + 1)) * np.sqrt(j)
            cf[j] = magnitude * np.exp(1j * angle) + np.conj(magnitude * np.exp(-1j * angle / (j + 1)))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_334(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            angle_part = np.sin(j * np.pi / 5) + np.cos(j * np.pi / 7 + np.angle(t1))
            magnitude_part = np.log(np.abs(rec[j] + imc[j]) + 1) * (np.abs(t1) + np.abs(t2)) / (j + 1)
            intricate_term = (rec[j]**3 - 2 * imc[j]**2) * np.cos(j * np.pi / 3)
            cf[j] = magnitude_part * np.exp(1j * angle_part) + np.conj(t1) * intricate_term
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_335(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_t1 = np.real(t1)
        imc_t1 = np.imag(t1)
        rec_t2 = np.real(t2)
        imc_t2 = np.imag(t2)
        for j in range(n):
            angle_part = np.sin(j * np.pi / 7) * np.cos(j * np.pi / 5) + np.angle(t1) * np.angle(t2)
            magnitude_part = np.log(np.abs(t1) + 1) * (j**2) / (1 + j) + np.abs(t2)**(1 + np.sin(j))
            phase_shift = np.exp(1j * (angle_part + np.imag(t1) * np.real(t2) / j))
            cf[j] = magnitude_part * phase_shift + np.conj(t1) * np.conj(t2) / (j + 1)
        for k in range(n):
            if k % 5 == 0:
                cf[k] *= (1 + 0.5 * np.cos(k * np.pi / 3))
            elif k % 3 == 0:
                cf[k] *= (1 + 0.3 * np.sin(k * np.pi / 4))
            else:
                cf[k] *= (1 + 0.2 * np.log(k + 1))
        cf = cf * np.prod(np.abs(cf))**(1/n) + np.sum(np.real(cf)) + 1j * np.sum(np.imag(cf))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_336(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            k = (j % 7) + 1
            r = np.real(t1) * np.sin(j * np.pi / 6) + np.real(t2) * np.cos(j * np.pi / 5)
            s = np.imag(t1) * np.cos(j * np.pi / 4) - np.imag(t2) * np.sin(j * np.pi / 3)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(r)) + np.abs(s)
            angle = np.angle(t1) * np.cos(r) + np.angle(t2) * np.sin(s)
            cf[j] = magnitude * np.exp(1j * angle) + np.conj(t1)**k * np.conj(t2)**k
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_337(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(35):
            phase = np.sin(j * np.real(t1)) + np.cos(j * np.imag(t2)) + np.log(np.real(t1) + 1) * np.angle(t2)
            magnitude = (np.abs(t1)**j + np.abs(t2)**(35 - j)) * (j % 7 + 1) / (j + 1)
            cf[j] = magnitude * (np.cos(phase) + 1j * np.sin(phase))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_338(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            real_part = np.real(t1) * np.cos(k * np.pi / 5) + np.real(t2) * np.sin(k * np.pi / 7)
            imag_part = np.imag(t1) * np.sin(k * np.pi / 6) - np.imag(t2) * np.cos(k * np.pi / 8)
            magnitude = np.sqrt(real_part**2 + imag_part**2) * np.log(np.abs(k) + 1) * (1 + np.sin(k))
            angle = np.arctan2(imag_part, real_part) + np.sin(k * np.angle(t1)) * np.cos(k * np.angle(t2))
            cf[k - 1] = magnitude * np.exp(1j * angle)
        for r in range(1, n + 1):
            cf[r - 1] += np.conj(cf[n - r]) * np.sin(r * np.pi / 10)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_339(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1,n+1):
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (np.sin(j * np.pi / 7) + np.cos(j * np.pi / 5)**2)
            cf[j-1] = magnitude * np.exp(1j * angle)
            for k in range(1, 6):
                cf[j-1] += (np.real(t1) * rec[k - 1] - np.imag(t2) * imc[k - 1]) * np.exp(1j * np.sin(k))
            if j % 3 == 0:
                cf[j-1] *= (1 + 1j * np.log(np.abs(rec[j-1] + imc[j-1]) + 1))
            else:
                cf[j-1] += np.conj(rec[j-1] - imc[j-1]) * np.cos(j * np.pi / 6)
        for r in range(1, 5):
            cf *= (1 + 0.1 * r * np.sin(r * np.pi / 4))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_340(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            mag_part = np.log(np.abs(t1) + j) * np.abs(np.sin(j * np.real(t2))) + np.sqrt(j) * np.abs(np.cos(j * np.imag(t1)))
            angle_part = np.angle(t1) * j + np.sin(j) + np.cos(j / 2)
            cf[j] = mag_part * np.exp(1j * angle_part) + np.conj(t2)**(j % 5 + 1)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_341(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            mag = (np.abs(t1) * np.log(j + 1) + np.abs(t2) * np.sqrt(j)) / (1 + j**1.3)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j / 2) + np.sin(j / 3 * np.pi)
            perturb = np.exp(1j * (np.sin(j / 4 * np.pi) + np.cos(j / 5 * np.pi)))
            cf[j] = mag * np.exp(1j * angle) * perturb
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_342(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, 36):
            k = j % 7 + 1
            r = np.real(t1) * np.log(j + 1)
            s = np.imag(t2) * np.sin(k * np.pi / 5)
            theta = np.angle(t1) * np.cos(k * np.pi / 3) + np.sin(k * np.pi / 4)
            magnitude = np.abs(t1)**k + np.log(np.abs(t2) + j)
            cf[j - 1] = (r + s * 1j) * np.exp(1j * theta) + np.conj(t1 + t2)**k * np.cos(j * np.angle(t1))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_343(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = np.real(t1)
        imc1 = np.imag(t1)
        rec2 = np.real(t2)
        imc2 = np.imag(t2)
        for j in range(n):
            mag_part = np.log(np.abs(rec1 * (j + 1) + imc2 / (j + 1))) + np.sum(np.sin(rec1 * (j + 1)) * np.cos(imc2 / (j + 1)))
            angle_part = np.angle(t1) * (j**0.5) + np.angle(t2) * np.sqrt(j) + np.sin(j * np.real(t1)) - np.cos(j * np.imag(t2))
            cf[j] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_344(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            mag = np.log(np.abs(rec_seq[j] * imc_seq[j]) + 1) * (1 + np.sin(j * np.pi / 3)) * (j % 4 + 1)
            ang = np.angle(t1) * np.cos(j * np.pi / 5) + np.angle(t2) * np.sin(j * np.pi / 7) + np.log(np.abs(rec_seq[j] + imc_seq[j]) + 1)
            cf[j] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_345(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            mag_part = np.log(np.abs(rec[j]) + 1) * np.prod(np.arange(1, j + 1)) / (j + 2)
            angle_part = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j / 3)
            cf[j] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
            cf[j] += np.conj(cf[j]) * np.sin(j * np.pi / 4)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_346(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            rec_part = np.real(t1) * np.sin(j / 2) + np.real(t2) * np.cos(j / 3)
            imc_part = np.imag(t1) * np.cos(j / 4) - np.imag(t2) * np.sin(j / 5)
            magnitude = np.log(np.abs(rec_part + imc_part) + 1) * (j**(1.2)) * (1 + np.sin(j * np.pi / 6))
            angle = np.angle(t1) * np.cos(j / 7) + np.angle(t2) * np.sin(j / 8)
            cf[j] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_347(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            mag = 0
            angle = 0
            for k in range(1, int(np.floor(j / 5)) + 2):
                mag += np.real(t1) * np.sin(j * k) * np.log(k + 1)
                angle += np.imag(t2) * np.cos(j + k) / (k + 1)
            for r in range(1, 4):
                mag *= (1 + np.real(t1) * 0.1 * r)
                angle += np.angle(t2) * 0.05 * r
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        cf *= np.exp(1j * np.sin(np.abs(t1) * np.arange(1, n + 1)))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_348(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            magnitude = np.log(np.abs(t1) + j) * np.sin(j * np.pi / 7) + np.cos(j * np.pi / 5) * np.abs(t2)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j] = magnitude * np.exp(1j * angle)
        for k in range(n):
            cf[k] += (np.real(t1)**(k % 5 + 1) - np.imag(t2)**(k % 3 + 1)) * np.exp(1j * (np.sin(k) + np.cos(k)))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_349(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        base_mag = np.abs(t1) * 2 + np.abs(t2)
        base_ang = np.angle(t1) - np.angle(t2)
        for j in range(n):
            mag = base_mag * j**1.2 + np.log(j + 1) * np.sqrt(n - j + 1)
            ang = base_ang * np.sin(j * np.pi / n) + np.cos(j * np.pi / (n / 2))
            cf[j] = mag * (np.cos(ang) + 1j * np.sin(ang))
        for k in range(n):
            increment = (np.real(t1) * np.real(t2) - np.imag(t1) * np.imag(t2)) + \
                         (np.real(t1) * np.imag(t2) + np.imag(t1) * np.real(t2)) * 1j
            angle_mod = np.sin(k) * np.cos(k) * np.angle(t1 + t2)
            cf[k] = cf[k] * (np.cos(angle_mod) + 1j * np.sin(angle_mod)) + np.conj(cf[k]) * np.log(np.abs(cf[k]) + 1)
        for r in range(1, int(np.floor(n / 5)) + 1):
            idx = (r * 7) % n
            adjustment = (np.real(t1)**2 - np.imag(t2)**2) + (2 * np.real(t1) * np.imag(t2)) * 1j
            cf[idx] += adjustment * np.sin(r)
        for m in range(n):
            cf[m] = cf[m] * np.exp(1j * np.sin(m * np.pi / 4)) + np.exp(1j * np.cos(m * np.pi / 3))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_350(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(35):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(j * np.pi / 6)
            mag_part2 = np.log(np.abs(t2) + j) * np.cos(j * np.pi / 8)
            magnitude = mag_part1 + mag_part2 + np.sqrt(j)
            angle_part1 = np.angle(t1) * np.sin(j / 3)
            angle_part2 = np.angle(t2) * np.cos(j / 4)
            angle = angle_part1 + angle_part2 + np.sin(j) * np.cos(j / 2)
            cf[j] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_351(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = np.real(t1)
        imc1 = np.imag(t1)
        rec2 = np.real(t2)
        imc2 = np.imag(t2)
        for j in range(n):
            mag_part = np.log(np.abs(rec1 * j**1.5 + imc2 / (j + 2)) + 1) * (1 + np.sin(j * np.pi / 4))
            angle_part = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 5) + np.sin(j * np.pi / 7)
            cf[j] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_352(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            sum_real = 0
            sum_imag = 0
            for k in range(1, j + 1):
                sum_real += np.real(t1)**k * np.cos(k * np.pi / j)
                sum_imag += np.imag(t2)**k * np.sin(k * np.pi / j)
            mag = np.log(np.abs(sum_real) + 1) * np.abs(t1)**(0.5) + np.log(np.abs(sum_imag) + 1) * np.abs(t2)**(0.3)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j / 2)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        for r in range(n):
            cf[r] *= (1 + 0.1 * np.sin(r) + 0.1j * np.cos(r))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_353(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            mag_variation = np.log(np.abs(rec[j] + imc[j]) + 1) * (1 + np.sin(j) + np.cos(j / 2))
            angle_variation = np.angle(rec[j] + 1j * imc[j]) + np.sin(rec[j] * np.pi / (j + 1)) - np.cos(imc[j] * np.pi / (j + 1))
            cf[j] = mag_variation * np.exp(1j * angle_variation)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_354(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            part1 = np.sin(j * np.pi / 5) * np.cos(j * np.angle(t1)) 
            part2 = np.log(np.abs(t2) + j) * np.sin(j * np.pi / 3)
            part3 = np.cos(j * np.pi / 4) + np.sin(j * np.pi / 6)
            magnitude = np.abs(t1)**(0.5 * j) + np.log(np.abs(j) + 1) * (j % 7 + 1)
            angle = part1 + part2 + part3
            cf[j] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_355(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(35):
            mag = 0
            angle = 0
            for k in range(1, 36):
                mag += (np.real(t1)**k * np.log(np.abs(t2) + k)) / (1 + k**2)
                angle += np.sin(k * np.angle(t1)) * np.cos(k * np.angle(t2))
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        for r in range(1, 36):
            cf[r - 1] *= np.exp(1j * (np.real(t1) * r - np.imag(t2) / (r + 1)))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_356(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(35):
            temp = 0
            for k in range(1, j + 1):
                temp += (np.real(t1)**k * np.sin(k * np.angle(t2))) + (np.imag(t2)**k * np.cos(k * np.angle(t1)))
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * temp
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j] = magnitude * np.exp(1j * angle)
        for r in range(35):
            cf[r] += (np.real(t1) - np.real(t2)) * np.sin(r) + (np.imag(t1) + np.imag(t2)) * np.cos(r)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_357(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            magnitude = np.log(np.abs(t1) + j) * np.sin(j / 2) + np.sqrt(np.abs(t2)) * np.cos(j / 3)
            angle = np.angle(t1) * j + np.angle(t2) * (n - j)
            cf[j] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        for k in range(n):
            cf[k] += np.real(t1) * np.sin(k) - np.imag(t2) * np.cos(k)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_358(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        real_diff = np.real(t2) - np.real(t1)
        im_diff = np.imag(t2) - np.imag(t1)
        for j in range(n):
            mag = 1 + np.abs(j - n / 2) * np.log(j + 1)
            angle = np.angle(t1) + (j / n) * np.angle(t2) + np.sin(j) * np.cos(j / 2)
            for k in range(1, 4):
                mag *= (1 + 0.1 * k * np.sin(j * k / n))
                angle += 0.5 * k * np.cos(j * k / n)
                for r in range(1, 3):
                    mag += 0.05 * r * np.log(j + r)
                    angle += 0.3 * r * np.sin(j * r / n)
            cf[j] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_359(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        t_conj = np.conj(t1) + np.conj(t2)
        for j in range(n):
            real_part = np.real(t1) * np.sin(j * np.pi / 7) + np.real(t2) * np.cos(j * np.pi / 5)
            imag_part = np.imag(t1) * np.cos(j * np.pi / 6) - np.imag(t2) * np.sin(j * np.pi / 4)
            magnitude = np.log(np.abs(t_conj) + j) * (np.abs(real_part) + np.abs(imag_part))
            angle = np.angle(t1) + np.angle(t_conj) * np.sin(j / 3) - np.angle(t2) * np.cos(j / 4)
            cf[j] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_360(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            magnitude = np.log(np.abs(rec[j] * imc[j]) + 1) * (j**np.sin(j)) * (1 + np.cos(j))
            angle = np.sin(2 * np.pi * rec[j]) + np.cos(3 * np.pi * imc[j]) + np.log(np.abs(rec[j] + imc[j]) + 1)
            cf[j] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_361(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        re1 = np.real(t1)
        im1 = np.imag(t1)
        re2 = np.real(t2)
        im2 = np.imag(t2)
        for j in range(n):
            mag = np.log(np.abs(t1) + j * re1) * (1 + np.sin(j * im2)) if j % 2 == 0 else np.log(np.abs(t2) + j * im1) * (1 + np.cos(j * re2))
            angle = np.sin(j * np.pi * re1 / n) + np.cos(j * np.pi * im2 / n) if j <= n / 2 else np.sin(j * np.pi * re2 / n) - np.cos(j * np.pi * im1 / n)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_362(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(n):
            k = (j * 4 + 5) % n + 1
            r = j // 6 + 1
            mag = np.log(np.abs(t1) + j**2) * np.abs(np.sin(j / 2 + k / 3)) + np.log(j + r)
            angle = np.angle(t1) * np.cos(j / (k + 1)) + np.angle(t2) * np.sin(j / (r + 2)) + np.real(t1) * np.imag(t2) / (j + 1)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_363(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            angle = np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2)) + np.sin(j**2 / n)
            magnitude = (np.log(np.abs(t1) + 1) * j) + (np.abs(t2)**0.5 * np.sqrt(j))
            cf[j - 1] = magnitude * np.exp(1j * angle) + np.conj(t1) * np.conj(t2) / (j + 1)
        for k in range(1, n // 2 + 1):
            cf[k - 1] += (np.real(t1) * np.real(t2) + np.imag(t1) * np.imag(t2)) * np.sin(k)
        for r in range(n - 4, n + 1):
            cf[r - 1] += np.prod([np.abs(t1), np.abs(t2), r]) * np.cos(r)
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_364(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag = np.log(np.abs(rec[j - 1] + imc[j - 1]) + 1) * (j**np.sin(j)) + np.sqrt(j) * np.abs(np.cos(j * np.pi / 3))
            ang = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j - 1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128).astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_365(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = len(cf)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(rec[j - 1] + imc[j - 1]) + 1) * j**(np.sin(j) + 1)
            angle_part = np.angle(t1) * np.cos(j / 3) + np.angle(t2) * np.sin(j / 4)
            cf[j - 1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        for k in range(1, n + 1):
            modifier = np.exp(1j * (np.sin(k * np.pi / 5) + np.cos(k * np.pi / 7)))
            cf[k - 1] = cf[k - 1] * modifier + np.conj(cf[max(0, k - 2)]) / (k + 1)
        for r in range(1, n + 1):
            cf[r - 1] += np.real(t1) * np.imag(t2) / (r + 1) + np.real(t2) * np.imag(t1) / (n - r + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_366(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            r = np.real(t1) * np.sin(j) + np.real(t2) * np.cos(j**2)
            im = np.imag(t1) * np.cos(j) - np.imag(t2) * np.sin(j**2)
            mag = np.log(np.abs(t1) + j) * np.sqrt(j) * (j % 5 + 1)
            angle = np.angle(t1) + np.angle(t2) + np.sin(j) * np.cos(j)
            cf[j - 1] = complex(r * np.cos(angle), im * np.sin(angle)) * mag
        for k in range(1, 36):
            cf[k - 1] += np.conj(cf[((k + 3) % 35)]) * np.sin(k / 2) - np.cos(k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_367(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            k = (j * 4 + 2) % n + 1
            r = (j + 3) % n + 1
            rec = np.real(t1) * np.sin(j) + np.real(t2) * np.cos(k)
            imc = np.imag(t1) * np.cos(r) - np.imag(t2) * np.sin(k)
            mag = np.log(np.abs(t1) + 1) * np.abs(rec) + np.sin(j) * np.cos(r) + np.prod([np.real(t1), np.imag(t2), j])
            angle = np.angle(t1) * k - np.angle(t2) * r + np.sin(j * np.pi / n)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_368(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.angle(t2)))
            angle = np.cos(j * np.real(t1)) + np.sin(j * np.imag(t2))**2
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, n + 1):
            cf[k - 1] += (np.real(t1)**k - np.imag(t2)**k) * np.exp(1j * np.angle(t1 + k * t2))
        for r in range(1, n + 1):
            cf[r - 1] *= (np.abs(t1 + r * t2)**(1 + r / 10)) * np.cos(r * np.angle(t1 * t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_369(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = (j * 2 + 5) % 10 + 1
            r = np.real(t1)**((j % 4) + 1) + np.real(t2) * np.log(np.abs(t1) + j)
            im_part = np.imag(t2) + np.sin(j / 3) * np.cos(k / 2)
            angle = np.angle(t1) * j - np.angle(t2) * k + np.sin(j) * np.cos(k)
            magnitude = np.abs(t1)**(1 + (j % 5)) + np.abs(t2)**(2 + (k % 3))
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1 + t2)**k
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_370(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.real(t1) + (np.real(t2) - np.real(t1)) * (np.arange(1, n + 1)) / n
        imc = np.imag(t1) + (np.imag(t2) - np.imag(t1)) * (np.arange(1, n + 1)) / n
        for j in range(1, n + 1):
            mag = np.log(1 + rec[j - 1]**2 + imc[j - 1]**2) * np.sin(j * np.angle(t1) + np.cos(j * np.angle(t2)))
            angle = np.angle(t1) * np.real(t2) / (j + 1) + np.angle(t2) * np.imag(t1) / (j + 2)
            cf[j - 1] = mag * np.exp(1j * angle) + np.conj(t1) * np.prod(np.arange(1, j + 1)) / (j + 3)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_371(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(t1) + j) * np.sqrt(j) + np.sin(j * np.angle(t2))**2
            angle_part = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j - 1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part)) + np.conj(t1)**j - np.log(np.abs(t2) + 1) * np.sin(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_372(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for k in range(1, n + 1):
            mag1 = np.log(np.abs(t1 * k) + 1) * np.sin(np.angle(t1) * k)
            mag2 = np.log(np.abs(t2 / k) + 1) * np.cos(np.angle(t2) / (k + 1))
            mag = mag1 + mag2
            angle = np.sin(rec[k - 1] * np.pi / (k + 2)) + np.cos(imc[k - 1] * np.pi / (k + 3))
            cf[k - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_373(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), 35)
        imc = np.linspace(np.imag(t1), np.imag(t2), 35)
        for j in range(1, 36):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.pi / 7) + np.cos(j * np.pi / 11) * np.real(t2)
            ang_part = np.angle(t1) + np.angle(t2) * j + np.sin(j * np.pi / 13)
            cf[j - 1] = (mag_part + np.imag(t1) * np.cos(j * np.pi / 5)) * np.exp(1j * ang_part) + np.conj(t2) * np.sin(j * np.pi / 17)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_374(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = np.real(t1) + np.real(t2) * j
            im = np.imag(t1) - np.imag(t2) * j
            mag = np.log(np.abs(t1) + j**2) * (1 + np.sin(j * np.pi / 5) * np.cos(j * np.pi / 7))
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_375(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            r = np.real(t1) * j**2 - np.real(t2) / (j + 1)
            im = np.imag(t2) * np.log(j + np.abs(t1)) + np.sin(j * np.angle(t1)) * np.cos(j * np.angle(t2))
            cf[j - 1] = (r + 1j * im) * np.exp(1j * (np.real(t1) * np.sin(j) + np.imag(t2) * np.cos(j)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_376(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag = 0
            angle = 0
            for k in range(1, j + 1):
                mag += (np.real(t1) * np.log(k + 1)) / (k**0.5)
                angle += np.sin(k * np.angle(t2)) + np.cos(k * np.real(t1))
            cf[j - 1] = mag * np.exp(1j * angle) + np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_377(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            angle = np.angle(t1) * np.log(j + 1) + np.sin(j) * np.angle(t2) / (j + 1)
            magnitude = np.abs(t1)**np.sqrt(j) + np.abs(t2)**(1 + 1/j) + np.log(np.abs(j - n/2) + 1)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1) * np.real(t2) / (j + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_378(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            term1 = np.exp(1j * np.sin(5 * np.pi * rec[j - 1]))
            term2 = np.exp(1j * np.cos(3 * np.pi * imc[j - 1]))
            term3 = np.log(np.abs(rec[j - 1] * imc[j - 1]) + 1)
            mag = term3 * (j**2) + np.sum(np.arange(1, j % 4 + 2))
            angle = np.angle(term1) + np.angle(term2) + np.sin(j * np.angle(t1 + t2))
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_379(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        for j in range(1, n + 1):
            magnitude = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.pi / 6)) + np.cos(j * np.pi / 4) * np.abs(t2)
            angle = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 5)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1)**(j % 7) * np.sin(j * np.angle(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_380(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            r = rec[j - 1]
            m = imc[j - 1]
            mag = np.log(np.abs(r**2 + m**2) + 1) * (j + 1)**(np.sin(r) + np.cos(m))
            angle = np.sin(j * r) + np.cos(j * m) + np.angle(t1) * np.sin(m) - np.angle(t2) * np.cos(r)
            cf[j - 1] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_381(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_t1 = np.real(t1)
        rec_t2 = np.real(t2)
        imc_t1 = np.imag(t1)
        imc_t2 = np.imag(t2)
        for j in range(1, n + 1):
            phase = np.sin(j * np.pi / 7) * np.cos(j * np.pi / 5)
            magnitude = np.log(np.abs(t1) + 1) * np.sin(j) + np.log(np.abs(t2) + 1) * np.cos(j)
            angle = np.angle(t1) * j**0.5 + np.angle(t2) / (j + 1)
            cf[j - 1] = magnitude * (np.cos(angle + phase) + 1j * np.sin(angle - phase)) + np.conj(t1)**j * np.real(t2) / (j + 2) + np.imag(t1 + t2) * np.cos(j * np.angle(t1)) * np.sin(j * np.angle(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_382(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            coeff_real = 0
            coeff_imag = 0
            for k in range(1, 6):
                coeff_real += np.real(t1)**k * np.sin(j * k * np.pi / 4) + np.real(t2)**(k / 2) * np.cos(j * k * np.pi / 3)
                coeff_imag += np.imag(t1)**k * np.cos(j * k * np.pi / 5) - np.imag(t2)**(k / 2) * np.sin(j * k * np.pi / 6)
            for r in range(1, 4):
                coeff_real += np.log(np.abs(t1 + r) + 1) * np.sin(j * r * np.pi / 7) * np.angle(t1 + r)
                coeff_imag += np.log(np.abs(t2 - r) + 1) * np.cos(j * r * np.pi / 8) * np.angle(t2 - r)
            cf[j - 1] = coeff_real + 1j * coeff_imag
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_383(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.real(t2)) + np.prod(np.arange(1, j + 1)) / (j + 1)
            angle_part = np.angle(t1) * np.cos(j * np.imag(t2)) + np.sin(j) * np.angle(t2)
            cf[j - 1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part)) + np.conj(t1)**j - np.conj(t2)**(n - j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_384(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            r = np.real(t1) + (np.real(t2) - np.real(t1)) * j / 35
            im = np.imag(t1) + (np.imag(t2) - np.imag(t1)) * j / 35
            magnitude = np.log(np.abs(t1) + j) * np.abs(np.sin(r * j)) + np.prod(np.arange(1, (j % 5) + 1))
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(im * j)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_385(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            real_part = np.real(t1)**k + np.real(t2)**(n - k)
            imag_part = np.imag(t1)**(k % 5 + 1) - np.imag(t2)**(k // 3 + 1)
            magnitude = (np.abs(t1) + np.abs(t2))**k * np.log(k + 1)
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k)
            cf[k - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_386(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            real_part = np.real(t1) * np.sin(j) + np.real(t2) * np.cos(j / 2)
            imag_part = np.imag(t1) * np.cos(j) - np.imag(t2) * np.sin(j / 2)
            magnitude = np.sqrt(real_part**2 + imag_part**2) * np.log(j + np.abs(t1) + np.abs(t2))
            angle = np.angle(t1) * np.sqrt(j) + np.angle(t2) * np.cos(j)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_387(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            j = k
            mag_part = np.log(np.abs(t1) + k) * np.sin(j) + np.cos(j) * np.log(np.abs(t2) + 1)
            angle_part = np.angle(t1) * j**0.5 + np.angle(t2) * np.log(j + 1) + np.sin(j * np.real(t1)) - np.cos(j * np.imag(t2))
            cf[j - 1] = mag_part * np.exp(1j * angle_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_388(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r_part = np.real(t1) * j + np.real(t2) / (j + 1)
            i_part = np.imag(t1) * np.sin(j) + np.imag(t2) * np.cos(j)
            mag = np.log(np.abs(t1) + j) * (j % 5 + 1)
            angle = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 4)
            cf[j - 1] = (r_part + 1j * i_part) * np.exp(1j * angle) * mag
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_389(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        for j in range(1, n + 1):
            coeff_real = 0
            coeff_imag = 0
            for k in range(1, 6):
                coeff_real += np.real(t1)**k * np.cos(k * np.angle(t2) + j)
                coeff_imag += np.imag(t2)**k * np.sin(k * np.angle(t1) + j)
            for r in range(1, 4):
                coeff_real += np.log(np.abs(t1 + t2) + 1) * np.real(t1) / r
                coeff_imag += np.log(np.abs(t1 - t2) + 1) * np.imag(t2) / r
            magnitude = np.sqrt(coeff_real**2 + coeff_imag**2) * (1 + j / n)
            angle = np.arctan2(coeff_imag, coeff_real) + np.sin(j) * np.cos(j / 2)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_390(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            angle = np.angle(t1) * j + np.sin(j * np.pi / 4) * np.cos(j * np.pi / 6)
            magnitude = np.abs(t1)**j + np.log(np.abs(t2) + 1) * (j % 5 + 1)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, n // 5 + 1):
            idx = np.random.choice(range(n))
            cf[idx] = cf[idx] * np.exp(1j * np.sin(k)) + np.conj(cf[idx])
        for r in range(1, n // 7 + 1):
            idx = np.random.choice(range(n))
            cf[idx] += np.abs(t1) * np.cos(r * np.pi / 3) - 1j * np.sin(r * np.pi / 4)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_391(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            magnitude = np.log(np.abs(rec[j - 1] + imc[j - 1]) + 1) * (j**0.5) * (1.2 if j % 2 == 0 else 0.8)
            angle = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 4) + np.sin(j) * np.cos(j / 2)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        for k in range(1, n + 1):
            cf[k - 1] += (np.prod(rec[:k] + imc[:k])**(1/k)) * np.exp(1j * np.angle(cf[k - 1]))
        for r in range(1, n + 1):
            cf[r - 1] *= np.conj(rec[r - 1] - imc[r - 1]) / (1 + np.abs(rec[r - 1] + imc[r - 1]))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_392(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        for j in range(1, n + 1):
            mag = 0
            ang = 0
            for k in range(1, 6):
                term1 = np.real(t1) * np.log(k + 1) * np.sin(j * k)
                term2 = np.imag(t2) * np.cos(j + k)
                mag += term1**2 + term2**2
                ang += np.angle(t1) * np.sin(k) - np.angle(t2) * np.cos(k)
            for r in range(1, 4):
                mag += np.abs(t1 + r) * np.sqrt(r) / (j + r)
                ang += np.sin(r * np.pi / j) * np.cos(r)
            varied_mag = mag * (1 + j / n)
            varied_ang = ang + np.log(np.abs(t1) + 1) - np.log(np.abs(t2) + 1)
            cf[j - 1] = varied_mag * (np.cos(varied_ang) + 1j * np.sin(varied_ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_393(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag = 0
            angle = 0
            for k in range(1, j + 1):
                mag += np.log(np.abs(t1) + k) * np.sin(k * np.real(t2)) / (k + 1)
                angle += np.angle(t1)**k * np.cos(k * np.imag(t2))
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_394(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = j
            r = np.real(t1) * np.sin(k) + np.real(t2) * np.cos(k**2)
            im = np.imag(t1) * np.cos(k / 3) - np.imag(t2) * np.sin(k / 4)
            mag = np.log(np.abs(t1) + np.abs(t2) + k) * (1 + np.sin(k) * np.cos(k))
            ang = np.angle(t1) * np.sin(k / 5) + np.angle(t2) * np.cos(k / 7)
            cf[j - 1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_395(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag_part1 = np.log(np.abs(rec[j - 1] * t1) + 1)
            mag_part2 = np.sum([np.cos(rec[j - 1]), np.sin(imc[j - 1])]) / (j ** 0.5)
            mag = mag_part1 * mag_part2 * (1 + np.sin(j * np.pi / 7))
            
            ang_part1 = np.angle(t1) * np.sin(j / 2)
            ang_part2 = np.angle(t2) * np.cos(j / 3)
            ang = ang_part1 + ang_part2 + np.log(np.abs(j) + 1)
            
            cf[j - 1] = mag * np.exp(1j * ang) + np.conj(t1 + t2) * np.sin(j) / (j + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_396(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        for j in range(1, n + 1):
            r_part = np.real(t1) * np.log(j + 1) + np.imag(t2) * np.sin(j * np.pi / 8)
            i_part = np.real(t2) * np.cos(j * np.pi / 7) - np.imag(t1) * np.sin(j * np.pi / 5)
            mag = np.sqrt(r_part**2 + i_part**2) + np.prod(np.arange(1, j + 1)) / (j + 2)
            temp = r_part + 1j * i_part
            theta = np.angle(temp) + np.cos(j * np.pi / 6)
            cf[j - 1] = mag * (np.cos(theta) + 1j * np.sin(theta))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_397(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            magnitude_part = np.log(np.abs(t1)**abs(j - n/2) + np.abs(t2)**(np.abs(j - n/3)) + 1)
            angle_part = np.sin(j * np.angle(t1)) * np.cos(j * np.angle(t2)) + np.sin(np.imag(t1) * np.pi / j)
            variation = (np.cos(np.angle(t1) * j) * np.sin(np.angle(t2) * (n - j)) if j % 3 == 0 
                         else np.sin(np.angle(t1) * (j + 1)) - np.cos(np.angle(t2) * (j + 2)))
            cf[j - 1] = magnitude_part * np.exp(1j * (angle_part + variation))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_398(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            r = np.real(t1) + (np.real(t2) * j) / 35
            d = np.imag(t1) - (np.imag(t2) * j) / 35
            mag = np.log(np.abs(r + 1j) + 1) * np.sin(j * np.pi / 4) + np.cos(j * np.pi / 6)
            ang = np.angle(d) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j - 1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_399(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            k = j % 7 + 1
            r = j // 5 + 1
            term1 = np.real(t1)**k * np.sin(k * np.pi / 3)
            term2 = np.imag(t2)**r * np.cos(r * np.pi / 4)
            magnitude = term1 + term2 + np.log(np.abs(t1) + np.abs(t2) + j)
            angle = np.angle(t1) * r - np.angle(t2) / k + np.sin(j) * np.cos(j)
            cf[j - 1] = magnitude * np.exp(1j * angle) + np.conj(t1 * t2) * (np.abs(t1) + np.abs(t2)) / j
            if j % 4 == 0:
                cf[j - 1] *= (np.sin(j * np.pi / 5) + np.cos(j * np.pi / 6))
            if j % 6 == 0:
                cf[j - 1] += np.real(t1)**2 - np.imag(t2)**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_400(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            term1 = np.sin(rec_seq[j - 1] * np.pi / n) * np.cos(imc_seq[j - 1] * np.pi / (2 * n))
            term2 = np.log(np.abs(rec_seq[j - 1] + imc_seq[j - 1]) + 1)
            term3 = np.abs(t1)**j / (1 + j)
            term4 = np.angle(t2) * j
            magnitude = term1 + term2 + term3
            angle = term4 + np.sin(j * imc_seq[j - 1]) - np.cos(j * rec_seq[j - 1])
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_401(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = np.real(t1)
        rec2 = np.real(t2)
        imc1 = np.imag(t1)
        imc2 = np.imag(t2)
        
        for j in range(1, n + 1):
            magnitude = (np.abs(t1) ** (j / 2)) * np.log(np.abs(j) + 1) + (np.abs(t2) ** (np.sqrt(j))) * np.sin(j)
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j / 2) + np.log(np.abs(j) + 1)
            cf[j - 1] = magnitude * np.exp(1j * angle)
        
        for k in range(1, n + 1):
            cf[k - 1] += (0.5 * np.real(t1) * np.sin(k)) + (0.3 * np.imag(t2) * np.cos(k))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_402(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            temp_mag = 0
            temp_angle = 0
            for k in range(1, j + 1):
                temp_mag += np.log(np.abs(t1) * k + 1) * np.sin(k * np.real(t2))
                temp_angle += np.cos(k * np.imag(t1)) * np.angle(t2 + k)
            cf[j - 1] = temp_mag * (np.cos(temp_angle) + np.sin(temp_angle) * 1j)
        for r in range(1, n + 1):
            cf[r - 1] = cf[r - 1] * (np.abs(t2)**(r / 2)) + np.conj(t1)**r
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_403(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            phase = np.sin(j * np.pi / 7) * np.cos(j * np.pi / 5) + np.log(np.abs(t1 + t2) + 1)
            magnitude = (np.real(t1)**j + np.imag(t2)**j) * np.sin(j) + np.cos(j * np.pi / 3)
            cf[j - 1] = magnitude * np.exp(1j * phase) + np.conj(magnitude * np.exp(1j * phase))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_404(t1, t2):
    try:
        n = 35
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag_part1 = np.real(t1) * np.log(j + 1)
            mag_part2 = np.abs(t2)**np.sin(j)
            mag_part3 = np.prod(1 + (j / 35))
            magnitude = mag_part1 + mag_part2 * mag_part3
            ang_part1 = np.angle(t1) * j
            ang_part2 = np.cos(j * np.pi / 7)
            angle = ang_part1 + ang_part2
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_405(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            phase = np.sin(k * np.angle(t1)) + np.cos(k * np.angle(t2))
            magnitude = np.log(np.abs(t1) + k) * np.exp(-k / (np.abs(t2) + 1)) + np.sqrt(k) * np.abs(t1 - t2)
            cf[k - 1] = magnitude * (np.cos(phase) + np.sin(phase) * 1j) + np.conj(t1) * np.sin(k) * np.cos(k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_406(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for r in range(1, n + 1):
            term1 = np.sin(r * np.pi / 8) * np.real(t1) * rec[r - 1]
            term2 = np.cos(r * np.pi / 6) * np.imag(t2) * imc[r - 1]
            term3 = np.log(np.abs(rec[r - 1] + imc[r - 1]) + 1)
            mag = term1 + term2 + term3
            angle = np.angle(t1) * np.sin(r * np.pi / 5) + np.angle(t2) * np.cos(r * np.pi / 7) + imc[r - 1] / 3
            cf[r - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_407(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n + 1):
            mag = np.abs(t1)**j / (j + 1) + np.abs(t2)**(n - j) * np.sin(j) + np.log(np.abs(rec[j - 1] + imc[j - 1]) + 1)
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j) + np.sin(j * np.pi / n)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, n + 1):
            cf[k - 1] += np.conj(cf[(k % n)]) * np.cos(k) - np.real(cf[k - 1]) * np.sin(k)
        for r in range(1, n + 1):
            if r % 3 == 0:
                cf[r - 1] *= (1 + 0.5 * np.sin(r)) 
            else:
                cf[r - 1] += 0.3 * np.imag(cf[r - 1]) * np.cos(r)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)



def poly_408(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_t1 = t1.real
        imc_t1 = t1.imag
        rec_t2 = t2.real
        imc_t2 = t2.imag
        
        for j in range(1, n + 1):
            angle_part = np.sin(j * np.pi * rec_t1) + np.cos(j * np.pi * imc_t2)
            magnitude_part = np.log(np.abs(j * rec_t2 + 1)) * np.sqrt(j) + np.abs(t1) ** 0.5
            phase_shift = np.angle(t1) * np.angle(t2) / j
            cf[j - 1] = (
                magnitude_part * np.exp(1j * (angle_part + phase_shift))
                + np.conj(t1) * np.sin(j * np.angle(t2))
                - np.cos(np.abs(t1))
            )
        
        for k in range(1, n // 2 + 1):
            idx = k * 2
            if idx <= n:
                perturbation = np.exp(1j * (np.sin(k) + np.cos(k)))
                cf[idx - 1] = cf[idx - 1] * perturbation + np.log(np.abs(cf[idx - 1]) + 1)
        
        for r in range(1, n + 1):
            scaling_factor = (r ** 2 + np.sqrt(r)) / (np.abs(t1) + np.abs(t2) + 1)
            cf[r - 1] = cf[r - 1] * scaling_factor
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_409(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            # Calculate the real part
            r_part = np.real(t1) * j**2 - np.real(t2) / (j + 1)
            # Calculate the imaginary part
            i_part = np.imag(t1) * np.log(np.abs(j) + 1) + np.imag(t2) * np.sin(j * np.pi / 7)
            # Calculate magnitude
            magnitude = np.sqrt(r_part**2 + i_part**2) * (1 + 0.1 * j)
            # Calculate angle
            angle = np.angle(t1) + np.angle(t2) + np.sin(j) - np.cos(j / 3)
            # Assign the complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_410(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            # Calculate magnitude parts
            mag_part1 = np.log(np.abs(t1) + j**1.5)
            mag_part2 = np.sin(j * np.real(t2)) * np.cos(j * np.imag(t1))
            magnitude = mag_part1 * (1 + mag_part2**2)
            # Calculate angle parts
            angle_part1 = np.angle(t1) + np.angle(t2) * j
            angle_part2 = np.log(np.abs(t1 * t2) + 1)
            angle = angle_part1 + angle_part2
            # Assign the complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_411(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            # Calculate real and imaginary components
            rec = np.real(t1) * j + np.real(t2) * (35 - j)
            imc = np.imag(t1) - np.imag(t2) * np.sin(j * np.pi / 8)
            # Calculate magnitude
            mag = np.log(np.abs(t1) + 1) * (np.sin(j * np.pi / 5) + np.cos(j * np.pi / 7)) * (1 + j % 6)
            # Calculate angle
            ang = np.angle(t1) * np.sin(j * np.pi / 9) + np.angle(t2) * np.cos(j * np.pi / 11) + np.sin(j * np.pi / 13)
            # Assign the complex coefficient
            cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_412(t1, t2):
    try:
        # x934
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        for j in range(1, n + 1):
            k = (j % 7) + 1
            r = np.floor(j / 5) + 1
            angle = (np.sin(k * np.real(t1) + r * np.imag(t2)) +
                     np.cos(k * np.imag(t1) - r * np.real(t2)) +
                     np.angle(t1) * np.angle(t2) / j)
            magnitude = (np.log(np.abs(t1) + 1) * (j**0.5 + r) +
                         np.abs(t2) * r**1.2)
            # Assign the complex coefficient using exponential form
            cf[j-1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_413(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            # Calculate angle and magnitude
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j / 3)
            magnitude = np.abs(t1)**j * np.log(np.abs(t2) + 1)**(n - j) * (j % 5 + 1)
            # Assign the complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        # Modify coefficients symmetrically
        for k in range(1, int(n/2) + 1):
            r = k**2 + np.sqrt(k)
            cf[k-1] *= np.exp(1j * r)
            cf[-k] *= np.conj(np.exp(1j * r))
        # Add variations based on index
        for r in range(1, n + 1):
            cf[r-1] += 0.1 * r * np.exp(-1j * r / n)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_414(t1, t2):
    try:
        # x934
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            j = (k % 5) + 1
            r = (np.real(t1) * np.sin(k) + np.real(t2) * np.cos(j)) / (j + k)
            mag = (np.log(np.abs(t1) + np.abs(t2) + k**1.5) * np.sin(r * np.pi / j) +
                   np.cos(r * np.pi / (k + 1))**2)
            angle = (np.angle(t1) * np.cos(j / (k + 1)) +
                     np.angle(t2) * np.sin(r))
            # Assign the complex coefficient
            cf[k-1] = mag * (np.cos(angle) + 1j * np.sin(angle)) * (1 + 0.05 * k**2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_415(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n +1):
            # Calculate magnitude and angle
            mag = (np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * np.pi / 7)) *
                   (1 + np.cos(j * np.pi / 5)))
            ang = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 4)
            # Assign the complex coefficient with additional terms
            cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang)) + \
                      np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_416(t1, t2):
    try:
        # x934
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            # Calculate magnitude parts
            mag_part1 = np.log(np.abs(t1) * j + 1)
            mag_part2 = np.sin(j * np.pi / 3) * np.abs(t2)
            mag = mag_part1 + mag_part2 * np.cos(j / 2)
            # Calculate angle parts
            angle_part1 = np.angle(t1) * np.cos(j / 4)
            angle_part2 = np.sin(j * np.pi / 5) * np.angle(t2)
            angle = angle_part1 + angle_part2 + np.sin(j) * np.cos(j / 3)
            # Assign the complex coefficient
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_417(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        r1 = np.real(t1)
        i1 = np.imag(t1)
        r2 = np.real(t2)
        i2 = np.imag(t2)
        for j in range(1, n +1):
            # Calculate magnitude and angle
            mag = (np.log(np.abs(r1 + j) + 1) * (j**1.5 + np.sin(j * r2)) *
                   (1 + np.abs(np.cos(j * i1))))
            ang = np.angle(t1) * np.sin(j * r2) + np.angle(t2) * np.cos(j * i1) + np.sin(j * i2)
            # Assign the complex coefficient
            cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_418(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n +1):
            sum_re = 0
            sum_im = 0
            for k in range(1, j +1):
                sum_re += rec_seq[k-1]**k * np.cos(k * np.pi / (j + 1))
                sum_im += imc_seq[k-1]**k * np.sin(k * np.pi / (j + 1))
            magnitude = np.log(1 + sum_re**2 + sum_im**2) * np.sin(j * np.pi / 5) + \
                        np.log(1 + sum_re * sum_im) * np.cos(j * np.pi / 7)
            angle = np.angle(t1) * sum_re - np.angle(t2) * sum_im + np.sin(j * np.pi / 3) - np.cos(j * np.pi / 7)
            cf[j-1] = magnitude * np.exp(1j * angle)
        # Additional modifications
        for k in range(1, n +1):
            cf[k-1] += (np.real(t1) * np.real(t2)) / (k + 1) * np.sin(k * np.pi / 6) + \
                       (np.imag(t1) + np.imag(t2)) * np.cos(k * np.pi / 8)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_419(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            # Calculate magnitude and angle parts
            mag_part = np.log(np.abs(t1) + np.abs(t2) + j) * np.sin(j * np.pi / 7) + np.cos(j * np.pi / 5)
            angle_part = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j / 2)
            # Assign the complex coefficient
            cf[j-1] = mag_part * np.exp(1j * angle_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_420(t1, t2):
    try:
        # x934
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        r1 = np.real(t1)
        i1 = np.imag(t1)
        r2 = np.real(t2)
        i2 = np.imag(t2)
        for j in range(1, n +1):
            # Calculate angle and magnitude
            angle_component = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude_component = np.abs(t1)**(j % 5 +1) + np.log(np.abs(t2) +1) * j
            phase_shift = np.sin(j * np.pi / 7) + np.cos(j * np.pi / 5)
            # Assign the complex coefficient with additional terms
            cf[j-1] = (magnitude_component * np.exp(1j * (angle_component + phase_shift)) + 
                       (np.conj(t1) * r2) / (j + 1) + 
                       (i1 + i2) * (j % 3))
        # Introduce region-based variations
        for k in range(1, n +1):
            if k <= n/3:
                cf[k-1] *= (1 + 0.5 * np.sin(k))
            elif k <= 2*n/3:
                cf[k-1] *= (1 + 0.3 * np.cos(k * 2))
            else:
                cf[k-1] *= (1 + 0.2 * np.sin(k * 3) * np.cos(k))
        # Add interactions between coefficients
        for r in range(1, n):
            cf[r-1] += 0.1 * cf[r] * np.exp(1j * np.angle(cf[r-1]))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_421(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n +1):
            if j <=10:
                # First segment
                mag = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.real(t1)) * np.cos(j * np.imag(t2)))
                angle = np.angle(t1) * np.sin(j / 2) + np.angle(t2) * np.cos(j / 3)
            elif j <=20:
                # Second segment
                mag = np.log(np.abs(t1 * t2) + j) * (1 + np.sin(j)**2 - np.cos(j)**2)
                angle = np.angle(t1 + t2) * np.sin(j / 4) + np.log(j + 1)
            else:
                # Third segment
                mag = np.log(np.abs(t1)**2 + np.abs(t2)**2 + j) * (1 + np.sin(j * np.real(t1) + np.cos(j * np.real(t2))))
                angle = np.angle(t1) * np.cos(j /5) + np.angle(t2) * np.sin(j /6)
            # Assign the complex coefficient
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_422(t1, t2):
    try:
        # x934
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            # Calculate real and imaginary parts
            r_part = np.real(t1) * np.sin(j * np.pi / 7) + np.real(t2) * np.cos(j * np.pi / 5)
            im_part = np.imag(t1) * np.cos(j * np.pi / 6) - np.imag(t2) * np.sin(j * np.pi / 8)
            # Calculate magnitude and angle
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * np.sin(j * np.pi /4) + np.cos(j * np.pi /3)
            angle = np.angle(t1) * np.cos(j /3) + np.angle(t2) * np.sin(j /4)
            # Assign the complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle)) + \
                      np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_423(t1, t2):
    try:
        # x934
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j <=10:
                mag = np.real(t1) * j**2 + np.log(np.abs(t2) + 1) * np.sin(j * np.real(t1))
                angle = np.angle(t1) + np.cos(j * np.imag(t2))
            elif j <=25:
                mag = np.exp(np.real(t2) / (j + 1)) + np.sqrt(j) * (np.real(t1) + np.real(t2))
                angle = np.sin(j) + np.angle(t2) * np.cos(j * np.real(t1))
            else:
                mag = np.log(np.abs(t1) + j) * np.exp(-np.real(t2) / j) + np.sin(j * np.imag(t1))
                angle = np.angle(t1 * t2) / j + np.cos(j * np.imag(t2))
            # Assign the complex coefficient
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_424(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n +1):
            # Calculate magnitude parts
            mag_part1 = np.real(t1) * j**2
            mag_part2 = np.log(np.abs(t2) + j) * np.sin(j * np.angle(t1))
            mag_part3 = np.cos(j * np.real(t2)) * np.sqrt(j)
            magnitude = mag_part1 + mag_part2 + mag_part3
            # Calculate angle parts
            angle_part1 = np.angle(t1) + np.sin(j * np.real(t1))
            angle_part2 = np.cos(j * np.imag(t2)) - np.angle(t2) / j
            angle = angle_part1 + angle_part2
            # Assign the complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_425(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n +1):
            angle_part = np.sin(j * np.pi /6) * np.cos(j * np.pi /8) + np.angle(t1) * np.log(j +1)
            magnitude_part = np.log(np.abs(t1) + j**2) * np.abs(np.cos(j)) + \
                             np.log(np.abs(t2) + j) * np.abs(np.sin(j / 2))
            cf[j-1] = (magnitude_part + np.real(t1) * np.real(t2) / (j +1)) * \
                      np.exp(1j * angle_part)
            if j %5 ==0:
                cf[j-1] += np.conj(cf[j-1])
            cf[j-1] *= (1 + 0.1 * np.sin(j))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_426(t1, t2):
    try:
        # x934
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        for k in range(1, n +1):
            j = 36 - k
            term1 = np.real(t1) * np.sin(k) * np.cos(j)
            term2 = np.imag(t2) * np.cos(k) * np.sin(j)
            term3 = np.log(np.abs(t1) + k)
            term4 = np.log(np.abs(t2) + j)
            angle = np.angle(t1) * np.sin(k / 2) + np.angle(t2) * np.cos(j / 3)
            magnitude = term1 + term2 + term3 - term4
            # Assign the complex coefficient
            cf[k-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_427(t1, t2):
    try:
        # x934
        cf = np.zeros(35, dtype=np.complex128)
        n =35
        for j in range(1, n +1):
            # Intermediate real and imaginary sequences
            rec = np.real(t1) + (np.real(t2) - np.real(t1)) * j / n
            imc = np.imag(t1) + (np.imag(t2) - np.imag(t1)) * j / n
            
            # Complex transformations (these variables are calculated but not used directly)
            conj_t1 = np.conj(t1)
            conj_t2 = np.conj(t2)
            
            # Magnitude calculations with intricate patterns
            mag_part1 = np.log(np.abs(rec) +1) * np.sin(j * np.pi /3)
            mag_part2 = np.log(np.abs(imc) +1) * np.cos(j * np.pi /4)
            magnitude = mag_part1 + mag_part2
            
            # Angle calculations with varying functions
            angle = np.angle(t1) * j + np.angle(t2) / (j +1) + np.sin(j * imc) * np.cos(j * rec)
            
            # Assigning the complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_428(t1, t2):
    try:
        # x934
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n +1):
            mag_part = np.log(np.abs(rec[j-1] * imc[j-1]) + 1) * (1 + np.sin(j * np.pi /5))
            angle_part = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j /3)
            cf[j-1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
            cf[j-1] += np.prod(rec[:j] + imc[:j])**(1/j)
            if j %5 ==0:
                cf[j-1] *= np.exp(1j * np.angle(cf[j-1]))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_429(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag_part = np.log(np.abs(t1) + j**2) * np.sin(j * np.real(t2)) + np.cos(j * np.imag(t1))
            angle_part = np.angle(t1) * j + np.angle(t2) * np.sqrt(j) + np.sin(j) * np.angle(t2)
            # Assign the complex coefficient
            cf[j-1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        # Modify coefficients based on additional terms
        for k in range(1, 36):
            r = np.real(t1) * k
            angle = np.angle(t2) * np.cos(k /4) + np.sin(k /6)
            cf[k-1] += (np.log(np.abs(t1 + t2) +1) * np.cos(angle) + 
                        1j * np.log(np.abs(t1 - t2) +1) * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_430(t1, t2):
    try:
        # x934
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n +1):
            mag = np.log(np.abs(rec[j-1] * imc[j-1]) +1) * (1 + np.sin(j * np.pi /4)) + \
                  (1 + np.cos(j * np.pi /3))
            ang = np.angle(t1) + np.angle(t2) + np.sin(j) - np.cos(j)
            # Assign the complex coefficient with additional terms
            cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang)) + \
                      np.conj(t1) * np.real(t2) * np.sin(j /2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_431(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n +1):
            term_real = np.sin(rec_seq[j-1] * j) + np.cos(imc_seq[j-1] * j**1.5)
            term_imag = np.log(np.abs(t1) + j) - np.sin(imc_seq[j-1] * np.pi / (j +1))
            mag = np.sqrt(term_real**2 + term_imag**2) * (1 + 0.1 * j)
            angle = np.angle(t1) * np.log(j +1) + np.cos(j * np.pi /7)
            # Assign the complex coefficient
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_432(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n +1):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(j * np.real(t2))
            mag_part2 = np.cos(j * np.imag(t1)) * (np.abs(t2)**(1 + j/10))
            magnitude = mag_part1 + mag_part2
            angle_part1 = np.angle(t1) * np.cos(j /5)
            angle_part2 = np.angle(t2) * np.sin(j /3)
            angle = angle_part1 + angle_part2
            # Assign the complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, n +1):
            cf[k-1] *= np.exp(1j * (np.real(t1) * k /n + np.imag(t2) * (n -k) /n))
        for r in range(1, n +1):
            cf[r-1] += np.conj(t1) * np.conj(t2) / (r +1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_433(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n +1):
            real_component = (np.real(t1)**j) * np.cos(j * np.angle(t2)) + \
                             np.sin(j * np.real(t1)) * np.log(np.abs(t2) + j)
            imag_component = np.imag(t1) * np.log(j +1) + \
                             np.cos(np.imag(t2)) * (np.abs(t1)**0.5)
            cf[j-1] = real_component + 1j * imag_component
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_434(t1, t2):
    try:
        # x934
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(np.real(t1), np.real(t2), n)
        imc_seq = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(1, n +1):
            magnitude = (np.log(np.abs(rec_seq[j-1]) + np.abs(imc_seq[j-1]) + j) *
                         (np.log(np.abs(t1) + np.abs(t2))**(j/10)))
            angle_part = np.sin(np.sin(j /2) * np.cos(j /3))
            angle = np.angle(t1) + np.angle(t2) + angle_part
            # Assign the complex coefficient with additional terms
            cf[j-1] = magnitude * np.exp(1j * angle) + \
                      np.conj(t1) * np.sin(j)**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_435(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n +1):
            mag_part = np.log(np.abs(t1) * j + np.abs(t2)**(n -j) +1)
            angle_part = np.angle(t1) * np.sin(j * np.pi /7) + np.angle(t2) * np.cos(j * np.pi /5)
            # Calculate real_sum and imag_sum
            real_sum = 0
            imag_sum =0
            for k in range(1, j +1):
                real_sum += np.real(t1)**k * np.real(t2)**(j -k)
                imag_sum += np.imag(t1)**k * np.imag(t2)**(j -k)
            intricate_mag = mag_part * (1 + np.sin(real_sum / (j +1)))
            intricate_angle = angle_part + np.cos(imag_sum / (j +1))
            # Assign the complex coefficient
            cf[j-1] = intricate_mag * (np.cos(intricate_angle) + 1j * np.sin(intricate_angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_436(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            # Calculate magnitude components
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(j / 3)
            mag_part2 = np.abs(t2)**(1 + (j % 4))
            magnitude = mag_part1 + mag_part2 * np.cos(j / 2)
            
            # Calculate angle components
            angle_part1 = np.angle(t1) * np.cos(j)
            angle_part2 = np.angle(t2) * np.sin(j / 2)
            angle = angle_part1 + angle_part2 + np.real(t1) * np.imag(t2) / j
            
            # Assign complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1)**((j % 5) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_437(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            # Linear interpolation between real and imaginary parts
            r = np.real(t1) + (np.real(t2) - np.real(t1)) * j / 35
            im = np.imag(t1) + (np.imag(t2) - np.imag(t1)) * j / 35
            
            # Calculate magnitude
            mag = np.log(np.abs(r * im) + 1) * (np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2))) + np.sqrt(r**2 + im**2)
            
            # Calculate angle
            angle = np.angle(t1) * np.sin(j / 3) - np.angle(t2) * np.cos(j / 4)
            
            # Assign complex coefficient
            cf[j-1] = mag * np.exp(1j * angle) + np.conj(t1) * np.cos(im * np.pi / 5) * np.sin(r * np.pi / 7)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_438(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        # First loop: initial coefficients
        for j in range(1, 36):
            # Calculate angle and magnitude
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j/2)
            magnitude = np.abs(t1)**(j%4 + 1) + np.log(np.abs(t2) + 1) * np.sqrt(j)
            # Assign to complex coefficient
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        
        # Second loop: modify coefficients
        for k in range(1, 36):
            cf[k-1] = cf[k-1] + np.conj(t1)**k * np.cos(k) - np.real(t2) * np.sin(k/3) + np.abs(t1 + t2)**(1 + (k % 5))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_439(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(np.real(t1), np.real(t2), n)
        imc = np.linspace(np.imag(t1), np.imag(t2), n)
        for j in range(n):
            r = rec[j]
            im = imc[j]
            mag = np.log(np.abs(r + im*1j) + 1) * (1 + np.sin((j+1) * np.pi / 4)) * (1 + np.cos((j+1) * np.pi / 3))
            angle = np.angle(r + im*1j) + np.sin((j+1) * np.pi / 5) - np.cos((j+1) * np.pi / 6)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_440(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        n = 35
        for j in range(1, n+1):
            # Calculate real and imaginary parts
            real_part = np.real(t1)**j * np.log(np.abs(j) + 1) + np.sin(j * np.real(t2)) * np.cos(j**2)
            imag_part = np.imag(t1) * j**0.5 + np.cos(j * np.imag(t2)) * np.log(np.abs(t1 + t2) + 1)
            # Assign to complex coefficient with scaling
            cf[j-1] = complex(real=real_part, imag=imag_part) * (1 + 0.1 * j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_441(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, num=n)
        imc_seq = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(n):
            mag_part = np.log(np.abs(rec_seq[j]) + np.abs(imc_seq[j]) + (j + 1)) * (np.abs(t1) + np.abs(t2)) ** ((j + 1) / 10)
            angle_part = np.sin((j + 1) * np.angle(t1)) * np.cos((j + 1) * np.angle(t2)) + np.log(np.abs(t1) + np.abs(t2) + (j + 1))
            cf[j] = mag_part * np.exp(1j * angle_part) + np.conj(mag_part * np.exp(-1j * angle_part))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_442(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(t1) + j**2) * np.sin(j * np.pi / 4) + np.cos(j * np.pi / 6)
            angle_part = np.angle(t2) + np.sin(j) * np.cos(j / 2) + np.log(np.abs(t1) + 1)
            cf[j - 1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        for k in range(1, n + 1):
            if k % 2 == 0:
                cf[k - 1] = cf[k - 1] * np.exp(1j * np.angle(t1) * k / 10)
            else:
                cf[k - 1] = cf[k - 1] * np.exp(-1j * np.angle(t2) * k / 15)
        for r in range(1, n + 1):
            cf[r - 1] = cf[r - 1] + np.conj(cf[n - r]) * (np.abs(t1) / (r + 1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_443(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        real_seq = np.linspace(t1.real, t2.real, num=n)
        imag_seq = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(1, n + 1):
            mag = np.log(np.abs(real_seq[j - 1] + t2) + 1) * np.sin(j * np.pi / 4) + np.cos(j * np.pi / 5) * np.abs(t1)
            angle = np.angle(t1) * j + np.sin(j * np.pi / 6) - np.cos(j * pi / 7)
            cf[j - 1] = mag * np.exp(1j * angle)
            for k in range(1, j + 1):
                cf[j - 1] += (t1.real * t2.real / (k + 1)) * np.exp(1j * (np.sin(k) - np.cos(k)))
        for r in range(1, n + 1):
            cf[r - 1] = cf[r - 1] * (1 + 0.05 * r**2) + np.conj(t2) * np.sin(r * np.pi / 8) - t1.real * np.cos(r * np.pi / 9)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_444(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag = np.log(np.abs(t1)**j + np.abs(t2)**(n - j)) + np.sin(j * t1.real) * np.cos(j * t2.imag)
            angle = np.angle(t1) * j - np.angle(t2) * (n - j) + np.sin(j) - np.cos(j)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_445(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            k = (j * 5 + 2) % 12 + 1
            r_part = t1.real * np.sin(j * np.pi / k) + t2.real * np.cos(j * np.pi / (k + 1))
            i_part = t1.imag * np.cos(j * np.pi / k) - t2.imag * np.sin(j * np.pi / (k + 1))
            magnitude = np.log(np.abs(t1) + j) * np.abs(np.sin(j * np.pi / 10))
            angle = np.angle(t1) * np.cos(j * np.pi / 8) + np.angle(t2) * np.sin(j * np.pi / 9)
            cf[j - 1] = magnitude * (r_part + 1j * i_part) * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def poly_446(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag = np.log(np.abs(t1)*j + np.abs(t2)/j + 1) * (1 + np.sin(j * np.pi / 4)) + np.prod([t1.real, t2.imag, j])
            angle = np.angle(t1)*j + np.angle(t2)*np.cos(j * np.pi / 5) + np.sin(j)**2
            cf[j-1] = mag * (np.cos(angle) + np.sin(angle)*1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_447(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            k = (j % 7) + 1
            r = (j % 5) + 2
            mag_part = np.log(np.abs(t1)*j + np.abs(t2)/k + 1) * np.sin(j * np.pi / r)
            angle_part = np.angle(t1)*np.cos(j / k) + np.angle(t2)*np.sin(j / r) + t1.real * t2.imag / (k + 1)
            cf[j-1] = mag_part * np.exp(1j * angle_part)
        for j in range(1, n+1):
            for r in range(1, j//3 +1):
                k = (j % 7) + 1
                cf[j-1] += (t1.real**r - t2.imag**k) * np.exp(1j * (np.angle(t1)*r - np.angle(t2)*k))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_448(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            rec = t1.real + (t2.real - t1.real)*j / n
            imc = t1.imag + (t2.imag - t1.imag)*j / n
            mag = np.log(np.abs(t1) + np.abs(t2) + j**3) * (1 + np.sin(j * np.pi / 5)) * (1 + np.cos(j * np.pi / 4))
            angle = np.angle(t1)*np.sin(j * np.pi / 3) + np.angle(t2)*np.cos(j * np.pi / 4) + np.sin(j * rec) * np.cos(j * imc)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_449(t1, t2):
    try:
        # x934
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            r = t1.real + j * t2.real
            k = t1.imag - j * t2.imag
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * np.pi / 7))
            angle = np.angle(t1)*np.sin(j / 3) + np.angle(t2)*np.cos(j / 5)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, n+1):
            r = t1.real * k
            angle = np.angle(t2)*np.cos(k / 4) + np.sin(k / 6)
            cf[k-1] += (np.log(np.abs(t1 + t2) + 1) * np.cos(angle) + 1j * np.log(np.abs(t1 - t2) + 1) * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_450(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            angle_part = np.sin(j * np.angle(t1) + np.cos(j * np.angle(t2)))
            mag_part = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + 0.1 * j)
            phase_shift = np.angle(t1) * np.cos(j / n * np.pi) - np.angle(t2) * np.sin(j / n * np.pi)
            cf[j-1] = mag_part * (np.cos(angle_part + phase_shift) + 1j * np.sin(angle_part - phase_shift))
        # Introduce variations using conjugates and nonlinear combinations
        for k in range(1, 6):
            idx = n - k
            if idx >=0:
                cf[idx] = cf[idx] * np.conj(t1)**((k % 3) + 1) + np.conj(t2)**(k % 4)
        # Apply modulation based on cumulative product
        cumulative = 1
        for r in range(1, n+1):
            cumulative *= (np.abs(t1) + np.abs(t2) + r)
            cf[r-1] += cumulative / (r + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_451(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) + j) * np.sqrt(j)
            mag_part2 = np.abs(t2)**(1 + np.sin(j))
            magnitude = mag_part1 + mag_part2
            phase_part1 = np.sin(j * t1.real) + np.cos(j * t2.imag)
            phase_part2 = np.angle(t1)*j - np.angle(t2)*(n - j)
            phase = phase_part1 * phase_part2 + np.sin(j / 3) * np.cos(j / 5)
            cf[j-1] = magnitude * np.exp(1j * phase)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_452(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n+1):
            temp = 0
            for j in range(1, k+1):
                temp += np.sin(j * t1.real) * np.cos(j * t2.imag) / j
            magnitude = np.log(np.abs(t1)*np.abs(t2) + k) * (1 + np.sin(k / 2) * 3)
            angle = np.angle(t1) + np.angle(t2) * np.log(k + 1) + temp
            cf[k-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_453(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            r_part = t1.real * j / n + t2.real * (n - j +1) /n
            i_part = np.sin(j * np.pi / n) * t1.imag - np.cos(j * np.pi / n) * t2.imag
            magnitude = np.log(np.abs(t1) + j) * (1 + np.abs(np.sin(j / 3)))
            angle = np.angle(t1)*np.cos(j) + np.angle(t2)*np.sin(j)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_454(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            mag = np.log(np.abs(rec_seq[j-1] * imc_seq[j-1]) + 1) * (1 + np.sin(j * np.pi / 4)) + np.prod(rec_seq[:j])**(1/j)
            angle = np.angle(t1)*j + np.sin(j * np.angle(t2)) + np.cos(j * imc_seq[j-1])
            cf[j-1] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_455(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag = np.log(np.abs(t1) + 1) * np.sin(j * t2.real) + np.cos(j * t1.imag)**2
            angle = np.angle(t1)*j + np.angle(t2)/(j +1) + np.sin(j * t1.real * t2.imag)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, 36):
            cf[k-1] = cf[k-1] * (1 + 0.05 * k * t1.real) / (1 + 0.03 * k * t2.imag)
            cf[k-1] += np.conj(cf[35 -k]) * np.exp(-0.1 * k)
        for r in range(1, 36):
            cf[r-1] = np.abs(cf[r-1])**np.sin(r * np.pi /17) * (np.cos(r * np.pi /23) + 1j * np.sin(r * np.pi /23))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_456(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            if j <=10:
                mag = np.log(np.abs(t1) + j**2) * np.abs(np.sin(j * t2.real) + np.cos(j * t1.imag))
                ang = np.angle(t1)**j + np.angle(t2)*np.sin(j)
            elif j <=20:
                mag = np.log(np.abs(t2) + j**1.5) * np.abs(np.cos(j * t1.real) - np.sin(j * t2.imag))
                ang = np.angle(t2)**j - np.angle(t1) * np.log(j + 1)
            else:
                mag = np.log(np.abs(t1 * t2) + j) * np.abs(np.sin(j * t1.real + np.cos(j * t2.real)))
                ang = np.angle(t1 + np.conj(t2)) * j + np.log(np.abs(t1 - t2) +1)
            cf[j-1] = mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_457(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(rec_seq[j-1]) + 1) * np.sin(j * np.pi /4)
            mag_part2 = np.abs(t2)**((j %5)+1)
            mag = mag_part1 + mag_part2 * np.cos(j * t1.real)
            angle_part1 = np.angle(t1)*np.cos(j /3)
            angle_part2 = np.sin(j * t2.imag) + np.cos(j * np.pi /6)
            angle = angle_part1 + angle_part2
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_458(t1, t2):
    try:
        cf = np.zeros(35,dtype=np.complex128)
        for j in range(1,36):
            k = (j%5)+1
            r = np.sqrt(j)*np.log(np.abs(t1)+np.abs(t2)+1)
            angle = np.angle(t1)*np.sin(k*j)+np.angle(t2)*np.cos(k*j)
            magnitude = ((np.real(t1)**k)+(np.imag(t2)**k))*(1+np.cos(j*np.pi/7))*(1+np.sin(j*np.pi/5))
            cf[j-1] = magnitude*(np.cos(angle)+1j*np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(35, dtype=np.complex128)

def poly_459(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n+1):
            temp_real = t1.real * np.log(np.abs(t1)*k + np.abs(t2) +1)
            temp_imag = t2.imag * np.sin(k) + np.cos(k * t1.real)
            temp_angle = (np.angle(t1 + t2)) / (k +1)
            magnitude = temp_real + temp_imag * temp_angle
            angle = np.sin(temp_real) + np.cos(temp_imag) * temp_angle
            cf[k-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_460(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            phase = np.angle(t1)*j + np.angle(t2)/(j +1) + np.sin(j * rec_seq[j-1]) - np.cos(j * imc_seq[j-1])
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (j**2 + np.sin(j)*np.cos(j))
            cf[j-1] = magnitude * (np.cos(phase) + 1j * np.sin(phase))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_461(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = t1.real
        imc1 = t1.imag
        rec2 = t2.real
        imc2 = t2.imag
        
        for j in range(1, n+1):
            magnitude = np.log(np.abs(t1) + j) * np.sin(j * np.pi /4) + np.cos(j * np.pi /3) * ((j %5) +1)
            angle = np.angle(t1)*np.cos(j/3) + np.angle(t2)*np.sin(j /5) + np.sin(j * np.pi /6)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        
        for k in range(1, n+1):
            cf[k-1] += (t1.real - t2.real) * np.sin(k * np.pi /7) + (t1.imag + t2.imag) * np.cos(k * np.pi /8)
        
        for r in range(1, n+1):
            cf[r-1] *= np.exp(1j * (np.sin(r) + np.cos(r)))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_462(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            mag = np.log(np.abs(rec_seq[j-1]*imc_seq[j-1] +1) +1) * (1 + np.sin(j)*np.cos(j/3))
            angle = np.angle(t1)*np.sin(j * np.pi /4) + np.angle(t2)*np.cos(j * np.pi /5)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_463(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t2)) + np.cos(j * np.angle(t1))
            ang_part = np.angle(t1)*j + np.angle(t2)*(n -j) + np.sin(j * t2.real) - np.cos(j * t1.imag)
            cf[j-1] = mag_part * np.exp(1j * ang_part)
        for k in range(1,6):
            for r in range(1, n+1):
                cf[r-1] += (t1.real * np.cos(k*r) + t2.imag * np.sin(k*r)) * np.exp(1j * (t2.real * r - t1.imag * k))
        for j in range(1, n+1):
            cf[j-1] *= (1 + 0.1 * j) / (1 + np.log(j +1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_464(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for k in range(1, n+1):
            mag_part = np.log(np.abs(t1) + k) * np.sin(k * np.pi /7) + np.cos(k * np.pi /5) * np.sqrt(k)
            angle_part = np.angle(t1)*np.sin(k) + np.angle(t2)*np.cos(k) + np.sin(k * t1.real) * np.cos(k * t2.imag)
            cf[k-1] = mag_part * np.exp(1j * angle_part)
        for r in range(1, n+1):
            cf[r-1] += (t1.real * t2.real / (r +1)) + 1j * (t1.imag - t2.imag) * np.sin(r)
        for j in range(1, n+1):
            cf[j-1] *= (1 + 0.1 * j) * np.exp(0.05j * j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_465(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2))
            ang_part = np.angle(t1)*j + np.angle(t2)*(n -j) + np.sin(j * t1.real) - np.cos(j * t2.imag)
            cf[j-1] = mag_part * np.exp(1j * ang_part)
        for k in range(1,6):
            for r in range(1, n+1):
                cf[r-1] += (t1.real * np.cos(k*r) + t2.imag * np.sin(k*r)) * np.exp(1j * (t2.real * r - t1.imag *k))
        for j in range(1, n+1):
            cf[j-1] *= (1 + 0.1 *j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_466(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            magnitude = np.log(np.abs(t1) + j**2) * np.abs(np.sin(j * np.angle(t1))) + np.sqrt(j) * np.cos(j * np.angle(t2))
            angle = np.angle(t1)*np.log(j +1) - t2.imag / (j +0.5) + np.sin(j * t1.real)*np.cos(j * t2.imag)
            cf[j-1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_467(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_sum = 0
            for k in range(1, j+1):
                mag_sum += np.log(np.abs(t1) + k) * np.sin(k * np.angle(t2))
            for r in range(1, n-j+1):
                mag_sum += np.log(np.abs(t2) + r) * np.cos(r * np.angle(t2))
            mag = np.log(mag_sum +1)
            angle = mag_sum / (j +1) + mag_sum / (n -j +1)
            cf[j-1] = mag * (np.cos(angle) + np.sin(angle)*1j)
        for j in range(1, n+1):
            cf[j-1] = cf[j-1] * (1 + 0.05 * j**2) + np.conj(cf[j-1]) * 0.02
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_467_old(t1, t2):
    n = 35
    cf = np.zeros(n, dtype=np.complex128)
    for j in range(n):
        mag = 0
        angle = 0
        for k in range(1, j + 2):  # R: 1:j -> Python: range(1, j+2)
            mag += np.log(np.abs(t1) + k) * np.sin(k * np.real(t2))
            angle += np.angle(t2**k) * np.cos(k / (j + 1))
        cf[j] = mag * (np.cos(angle) + np.sin(angle) * 1j)

    for j in range(n):
        cf[j] = cf[j] * (1 + 0.05 * (j + 1)**2) + np.conj(cf[j]) * 0.02

    return cf.astype(np.complex128)

def poly_468(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            if j <=10:
                mag = np.log(np.abs(t1) + j**2) * np.abs(np.sin(j * t2.real)) + np.cos(j * t1.imag)**2
                ang = np.angle(t1)**j + np.angle(t2)*np.sin(j)
            elif j <=20:
                mag = np.log(np.abs(t2) + j**1.5) * np.abs(np.cos(j * t1.real) - np.sin(j * t2.imag))
                ang = np.angle(t2)**j - np.angle(t1)*np.log(j+1)
            else:
                mag = np.log(np.abs(t1 * t2) + j) * np.abs(np.sin(j * t1.real + np.cos(j * t2.real)))
                ang = np.angle(t1 + np.conj(t2))*j + np.log(np.abs(t1 - t2) +1)
            cf[j-1] = mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_469(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * np.angle(t1)) * np.cos(j * np.angle(t2)))
            angle = np.angle(t1)**j - np.angle(t2)**(j %4) + np.sin(j * t1.real) - np.cos(j * t2.imag)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
            for k in range(1,4):
                mag = mag * (1 + 0.1 * np.sin(k * t1.real + 0.0))  # Adjusted to numpy
                angle = angle + 0.5 * np.angle(t1)**k - 0.3 * np.angle(t2)**k
                cf[j-1] += mag * (np.cos(angle) + 1j * np.sin(angle))
            cf[j-1] = cf[j-1] * (1 + 0.05 * j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_470(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for k in range(1, n+1):
            r = rec_seq[k-1]
            im = imc_seq[k-1]
            mag = np.log(np.abs(r) + 1)*np.abs(t1)**0.5 + np.sin(r * k)*np.cos(im / (k +1)) + (k %3 +1)*np.abs(t2)
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k * np.pi /4) + np.sin(im * k /2)
            cf[k-1] = mag * (np.cos(angle) + np.sin(angle)*1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_471(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j) + np.cos(j * t2.real)
            angle_part = np.angle(t1)*np.sqrt(j) + t2.imag / (j +1)
            cf[j-1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        for k in range(1, n+1):
            cf[k-1] += (t2.real - t1.imag) * np.exp(1j * np.log(k +1)) * np.cos(k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_472(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            mag = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.pi /7))
            ang = np.angle(t1)*np.cos(j * np.pi /5) + np.angle(t2)*np.sin(j * np.pi /3)
            cf[j-1] = mag * np.exp(1j * ang) + (t1.real + t2.real)/(j +1)
        for k in range(1, n+1):
            cf[k-1] += np.conj(t1)**k - np.conj(t2)**(n -k +1)
        for r in range(1, n+1):
            cf[r-1] *= (1 + 0.1 * np.cos(r * np.angle(t1)) * np.sin(r * np.angle(t2)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_473(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            mag = np.log(np.abs(rec_seq[j-1]*imc_seq[j-1] +1)**1) * (1 + np.sin(j * np.pi /4)*np.cos(j * np.pi /6))
            angle = np.angle(t1)*np.sin(j /2) + np.angle(t2)*np.cos(j /3) + np.log(np.abs(j) +1)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_474(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            k = j**2
            r = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t2))
            angle = np.angle(t1)*np.sin(k) - np.angle(t2)*np.cos(k) + np.log(np.abs(t2) +1)
            magnitude = (t1.real * np.cos(k) + t2.imag * np.sin(k)) * (np.abs(t1)**2 / (k +1))
            cf[j-1] = magnitude * (np.cos(angle) + np.sin(angle)*1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_475(t1, t2):
    try:
        n =40
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = t1.real**j + np.log(np.abs(t2) +j) + np.prod(np.arange(1, j+1))**(1/3)
            angle = np.angle(t1)*np.sin(j * np.pi /6) + np.angle(t2)*np.cos(j * np.pi /4)
            cf[j-1] = mag * np.exp(1j * angle) + np.conj(t1)*np.sin(j/2) - np.conj(t2)*np.cos(j/3)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_476(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(j * t1.real)
            mag_part2 = np.abs(t2)**0.5 * np.cos(j /3)
            magnitude = mag_part1 + mag_part2 + j**2
            angle_part1 = np.angle(t1)*np.cos(j * np.pi /4)
            angle_part2 = np.angle(t2)*np.sin(j * np.pi /5)
            angle = angle_part1 + angle_part2 + np.sin(j)
            cf[j-1] = magnitude * (np.cos(angle) + np.sin(angle)*1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_477(t1, t2):
    try:
        n =40
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            r = t1.real + t2.real * j
            i_part = t1.imag - t2.imag * j
            phase = np.sin(r) * np.cos(i_part) + np.log(np.abs(t1) + j)
            magnitude = np.abs(t1)**0.5 * np.abs(t2)**0.3 * j**np.sin(j) + np.cos(j * np.angle(t2))
            cf[j-1] = magnitude * np.exp(1j * phase)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_478(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) + j)
            mag_part2 = np.sin(j * t1.real) * np.cos(j / (t1.imag +1))
            magnitude = mag_part1 * mag_part2 + np.prod(np.arange(1, j+1))**0.5
            angle_part1 = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j)
            angle_part2 = np.sin(j * t1.real) - np.cos(j * t2.imag)
            angle = angle_part1 + angle_part2
            cf[j-1] = magnitude * np.exp(1j * angle) + np.conj(t1)*np.sin(j) - np.conj(t2)*np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_479(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(rec_seq[j-1]) +1) * np.sin(j * np.pi /4)
            mag_part2 = np.abs(t2)**((j %5) +1)
            mag = mag_part1 + mag_part2 * np.cos(j * t1.real)
            angle_part1 = np.angle(t1)*np.cos(j /3)
            angle_part2 = np.sin(j * t2.imag) + np.cos(j * np.pi /6)
            angle = angle_part1 + angle_part2
            cf[j-1] = mag * (np.cos(angle) + np.sin(angle)*1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_480(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(j * t1.real)
            mag_part2 = np.log(np.abs(t2) + j) * np.cos(j * t1.imag)
            magnitude = mag_part1 + mag_part2 + j**0.5
            angle_part1 = np.angle(t1)*np.cos(j / (t1.real +1))
            angle_part2 = np.angle(t2)*np.sin(j / (t2.imag +1))
            angle = angle_part1 - angle_part2 + np.sin(j * np.pi /6)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_481(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            term1 = np.exp(1j * np.sin(5 * np.pi * rec_seq[j-1])) * (t1.real + j)
            term2 = np.exp(1j * np.cos(3 * np.pi * imc_seq[j-1])) * (t2.imag + j**2)
            cf[j-1] = term1 + term2 + np.log(np.abs(t1)*np.abs(t2)+1)
        for k in range(1, n+1):
            cf[k-1] *= (1 + 0.05 * k * np.sin(cf[k-1].real) + 0.05j * k * np.cos(cf[k-1].imag))
        for r in range(1, n+1):
            cf[r-1] += np.conj(cf[r-1]) * np.sin(r * t1.real) * np.cos(r * t2.imag)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_482(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) + j)
            mag_part2 = np.sin(j * t2.real) * np.cos(j / (t1.imag +1))
            magnitude = mag_part1 * mag_part2 + np.prod(np.arange(1, j+1))**0.5
            angle_part1 = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j)
            angle_part2 = np.sin(j * t1.real) - np.cos(j * t2.imag)
            angle = angle_part1 + angle_part2
            cf[j-1] = magnitude * np.exp(1j * angle) + np.conj(t1)*np.sin(j) - np.conj(t2)*np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_483(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, n+1):
            part1 = r1**j * np.sin(j * np.angle(t2))
            part2 = i2**(n -j) * np.cos(j * np.abs(t1))
            part3 = np.log(np.abs(t1) + np.abs(t2) + j)
            part4 = np.prod([r1 + j, i2 +j, np.log(np.abs(t1)+1)])
            magnitude = part1 * part2 + part3 * part4
            angle = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j) + np.log(np.abs(t1)+1)/j
            cf[j-1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_484(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = np.log(np.abs(t1) + j) * np.sqrt(j) + np.sin(j * t2.real)**2 + np.cos(j * t1.imag / (j +1))
            angle = np.angle(t1)*j + np.sin(j * t1.real * t2.real) - np.cos(j * t2.imag)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_484_old(t1, t2):
    n = 35
    cf = np.zeros(n, dtype=np.complex128)
    for j in range(1, n + 1):
        mag = (
            np.log(np.abs(t1) + j) * np.sqrt(j)
            + np.sin(j * t2.real) ** 2
            + np.cos(j * t1.imag / (j + 1))
        )
        angle = (
            np.angle(t1) * j
            + np.sin(j * t1.real * t2.real)
            - np.cos(j * t2.imag)
        )
        cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
    return cf.astype(np.complex128)

def poly_485(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            phase = np.sin(j * np.pi /4) * np.cos(j * np.pi /3) + np.log(np.abs(rec_seq[j-1] + imc_seq[j-1]) +1)
            magnitude = np.sqrt(rec_seq[j-1]**2 + imc_seq[j-1]**2)**(1 + 0.1 *j) * np.abs(np.sin(j)) + np.abs(np.cos(j /2))
            cf[j-1] = magnitude * (np.cos(phase) + 1j * np.sin(phase))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_486(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_real = np.log(np.abs(t1) + np.abs(t2) + j) * np.abs(np.sin(j * t1.real) + np.cos(j * t2.imag))
            mag_imag = np.log(np.abs(t1) + np.abs(t2) + j) * np.abs(np.sin(j * t1.imag) - np.cos(j * t2.imag))
            angle_real = np.angle(t1)*np.sin(j /n * np.pi) + np.angle(t2)*np.cos(j /n * np.pi)
            angle_imag = np.angle(t1)*np.cos(j /n * np.pi) - np.angle(t2)*np.sin(j /n * np.pi)
            cf[j-1] = (mag_real * np.cos(angle_real) + mag_imag * np.sin(angle_imag)) +\
                      1j * (mag_real * np.sin(angle_real) - mag_imag * np.cos(angle_imag))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_487(t1, t2):
    try:
        n =35
        cf = np.zeros(40, dtype=np.complex128)
        for j in range(1, n+1):
            rec_seq = np.linspace(t1.real, t2.real, n)
            imc_seq = np.linspace(t1.imag, t2.imag, n)
            mag_part1 = np.log(np.abs(rec_seq[j-1]) +1) * np.sin(j * np.pi /4)
            mag_part2 = np.abs(t2)**((j %5) +1)
            mag = mag_part1 + mag_part2 * np.cos(j * t1.real)
            angle_part1 = np.angle(t1)*np.cos(j /3)
            angle_part2 = np.sin(j * t2.imag) + np.cos(j * np.pi /6)
            angle = angle_part1 + angle_part2
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, n+1):
            if k >1 and k <n:
                cf[k-1] += 0.5 * (cf[k-2] * np.conj(cf[k])) * np.cos(k * np.pi /n)
            elif k ==1:
                cf[k-1] += 0.3 * np.conj(cf[k]) * np.sin(k * np.pi /n)
            else:
                cf[k-1] += 0.3 * np.conj(cf[k-2]) * np.sin(k * np.pi /n)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_488(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, n+1):
            mag = np.log(np.abs(t1) + j) * np.abs(np.sin(j * t1.real)) + np.log(np.abs(t2) + j) * np.abs(np.cos(j * t1.imag / (j +1)))
            angle_part1 = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j)
            angle_part2 = np.sin(j * t1.real) - np.cos(j * t2.imag)
            angle = angle_part1 + angle_part2
            cf[j-1] = mag * np.exp(1j * angle) + np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_489(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            sum_mag =0
            for k in range(1, j+1):
                sum_mag += t1.real**k * np.sin(k * np.angle(t1))
            for r in range(1, (n-j)+1):
                sum_mag += t2.imag**r * np.cos(r * np.angle(t2))
            mag = np.log(sum_mag +1)
            angle = sum_mag / (j +1) + sum_mag / (n -j +1)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_490(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = t1.real**((j %5)+1) + np.abs(t2)**(np.floor(j /7)+1) + np.log(j +1)*np.sin(j * np.pi /4)
            angle = np.angle(t1)*np.cos(j * np.pi /6) + np.angle(t2)*np.sin(j * np.pi /8)
            cf[j-1] = mag * np.exp(1j * angle)
        for k in range(1, n+1):
            cf[k-1] += np.conj(t1) * np.cos(k * np.pi /5) + np.conj(t2) * np.sin(k * np.pi /3)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_491(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            r = rec_seq[j-1]
            i_part = t1.imag * np.sin(j) + t2.imag * np.cos(j)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (j**1.5 + np.prod(np.arange(1, (j %5)+2)))
            angle = np.angle(t1)*np.cos(j * np.pi /n) + np.angle(t2)*np.sin(j * np.pi /n)
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_492(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            real_part = t1.real * np.sin(j * np.pi /7) + t2.real * np.log(j +1)
            imag_part = t1.imag * np.cos(j**2 /5) - t2.imag * np.exp(-j /10)
            magnitude = (np.abs(t1) + np.abs(t2)) * (j**1.5 + (n -j)**1.2)
            angle = np.angle(t1)*np.sqrt(j) + np.angle(t2)*np.sin(j * np.pi /3)
            cf[j-1] = (real_part +1j * imag_part) * (np.cos(angle) + np.sin(angle)*1j) * magnitude
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_493(t1, t2):
    try:
        n =40
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = np.log(np.abs(t1) + np.abs(t2) +j) * (1 + np.sin(j) + np.cos(j /3))
            angle = np.angle(t1)*j + np.angle(t2)*np.sin(j /2)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_494(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            rec = t1.real * j
            imc = t2.imag / j
            mag = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.pi /7)) * (1 + np.cos(j * np.pi /5))
            angle = np.angle(t1)*np.sin(j /3) + np.angle(t2)*np.cos(j /4) + np.sin(j /2)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_495(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = t1.real
        imc1 = t1.imag
        rec2 = t2.real
        imc2 = t2.imag
        for j in range(1, n+1):
            mag = (np.sin(rec1 * j) + np.cos(imc2 * j**1.2)) * np.log(1 +j) + np.abs(t1)**0.5 * np.abs(t2)**0.3
            ang = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j**2)
            cf[j-1] = mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_496(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = np.log(np.abs(t1)*j +1) * np.sin(j * np.pi * t2.real / (j +1)) +\
                  np.log(np.abs(t2)*(n -j +1) +1)*np.cos(j * np.pi * t1.imag / (j +1))
            ang = np.angle(t1)*np.sin(j /2) + np.angle(t2)*np.cos(j /3) + np.log(j +1)
            cf[j-1] = mag * (np.cos(ang) +1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_497(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(rec_seq[j-1] + imc_seq[j-1]) +1)
            mag_part2 =1 + np.sin(j * np.pi /6)*np.cos(j * np.pi /4)
            magnitude = mag_part1 * mag_part2 * (1 + np.prod([j, rec_seq[j-1], imc_seq[j-1]])**(1/3))
            ang_part1 = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j)
            ang_part2 = np.sin(j * np.pi /5)*np.cos(j * np.pi /7)
            angle = ang_part1 + ang_part2
            cf[j-1] = magnitude * np.exp(1j * angle)
        for k in range(1, n+1):
            if k >1 and k <n:
                cf[k-1] += 0.5 * (cf[k-2] * np.conj(cf[k])) * np.cos(k * np.pi /n)
            elif k ==1:
                cf[k-1] += 0.3 * np.conj(cf[k]) * np.sin(k * np.pi /n)
            else:
                cf[k-1] += 0.3 * np.conj(cf[k-2]) * np.sin(k * np.pi /n)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_498(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            magnitude = np.log(np.abs(t1)*j + np.abs(t2)/(j +1) +1)
            angle = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j)
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_499(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) + j**1.5) * np.sin(j * np.pi /6)
            mag_part2 = np.abs(t2)/(j +2) + np.cos(j * np.pi /4)
            magnitude = mag_part1 + mag_part2 * np.exp(-j /10)
            angle_part1 = np.angle(t1)*np.cos(j /3)
            angle_part2 = np.angle(t2)*np.sin(j /5) + np.sin(j**2 /7)
            angle = angle_part1 + angle_part2
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_500(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            angle = np.sin(j * np.angle(t1)) * np.cos(j * np.angle(t2)) + np.sin(j**2 * np.angle(t1 + t2))
            magnitude = (np.abs(t1)**j + np.abs(t2)**(n -j)) * np.log(j + np.abs(t1 - t2)) / (1 + (j %5))
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle)) + np.conj(t1)*np.sin(j * t2.imag)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_501(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = np.abs(t1)**((j %5)+1) * np.log(np.abs(t2) +j) * np.sin(j *0.5) + np.angle(t2)*np.cos(j *0.3)
            angle = np.angle(t1)*np.sin(j *0.5) + np.angle(t2)*np.cos(j *0.3)
            cf[j-1] = mag * np.exp(1j * angle)
            if j %3 ==1:
                cf[j-1] += np.conj(t1)*np.sin(j * t2.real) - np.conj(t2)*np.cos(j * t1.imag)
            elif j%3 ==2:
                cf[j-1] += t1.real * t2.imag * np.sin(j)
            else:
                cf[j-1] += t2.real * t1.imag * np.cos(j)
        for k in range(1, n+1):
            cf[k-1] = cf[k-1] * (1 +0.05 *k) + 0.02 * np.sin(k * np.angle(t1)) * np.cos(k * np.angle(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_502(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            phase_r = np.sin(j * t1.real) + np.cos(j * t2.real)
            phase_i = np.cos(j * t1.imag) - np.sin(j * t2.imag)
            magnitude = np.log(np.abs(t1)**j + np.abs(t2)**(n -j) +1)
            angle = np.angle(t1)*j - np.angle(t2)*(n -j) + phase_r * phase_i
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_503(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = t1.real
        imc1 = t1.imag
        rec2 = t2.real
        imc2 = t2.imag
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) +j) * (1 + np.sin(j)*np.cos(j))
            mag_part2 = np.angle(t1)*j + np.angle(t2)/(j +1) + np.sin(j * rec1) - np.cos(j *imc2)
            magnitude = mag_part1 * mag_part2
            angle = np.angle(t1)*j + np.angle(t2)/(j +1) + np.sin(j * rec1) - np.cos(j * imc2)
            cf[j-1] = magnitude * np.exp(1j * angle) + np.conj(t1)*np.sin(j) + np.conj(t2)*np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_504(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            magnitude = np.log(np.abs(rec_seq[j-1] + imc_seq[j-1]) +1) * (np.abs(t1)**j + np.abs(t2)**(n -j))
            angle = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j/2)
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        for k in range(1, np.floor(n/2)+1):
            if k <=n:
                cf[k-1] += np.prod(rec_seq[:k]) * np.conj(t2)**k
                cf[n -k] += np.sum(imc_seq[:k]) * np.sin(np.abs(t1)*k) * np.cos(np.abs(t2)/(k +1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_505(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = t1.real
        imc1 = t1.imag
        rec2 = t2.real
        imc2 = t2.imag
        for j in range(1, n+1):
            mag = (np.sin(rec1 * j) + np.cos(imc2 * j**1.2)) * np.log(1 +j) + np.abs(t1)**0.5 * np.abs(t2)**0.3
            ang = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j**2)
            cf[j-1] = mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_506(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            r = t1.real + j * t2.real
            im = t1.imag - j * t2.imag
            mag = np.log(np.abs(t1) +j) * np.abs(np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2)))
            angle = np.angle(t1)*j + np.angle(t2)*(n -j)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        for k in range(1, n+1):
            cf[k-1] += np.conj(cf[k-1]) * np.sin(k * t1.real) / (1 +k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_507(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            mag = np.log(np.abs(rec_seq[j-1] * imc_seq[j-1]) +1) * ((j %4) +1)
            angle = np.sin(j * np.pi * t1.real) + np.cos(j * np.pi * t2.imag) + np.abs(t1.real)**0.5 * np.abs(t2.imag)**0.3
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        for k in range(1, n+1):
            cf[k-1] = cf[k-1] * (1 +0.5 * np.conj(cf[max(0, k -3)]) ) + 0.3 * np.sin(k)*np.cos(k)
        for r in range(1, n+1):
            cf[r-1] += cf[r-1] * (1 + 0.5 * np.conj(cf[max(0, r -2)])) + 0.3 * np.sin(r)*np.cos(r)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_508(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            k =j**2
            angle_part = np.angle(t1)*np.sin(j * np.pi /4) + np.angle(t2)*np.cos(j * np.pi /6)
            magnitude_part = np.log(np.abs(t1) + np.abs(t2) +j) * (1 + np.sin(j /3)) * (1 + np.cos(j /5))
            perturbation = np.conj(t1)**0.5 * np.sin(j) + np.conj(t2)**0.3 * np.cos(j)
            cf[j-1] = magnitude_part * np.exp(1j * angle_part) + 0.1 * perturbation
        for k in range(1,6):
            for r in range(1,8):
                index = (k -1)*7 + r
                if index <=n:
                    cf[index-1] = cf[index-1] * (1 +0.05 * np.sin(k * r)) +0.02 * np.cos(k +r)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_509(t1, t2):
    try:
        n =35
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            mag_part = np.log(np.abs(t1) + np.abs(t2) +j) * (1 + np.sin(j /4) * np.cos(j /6))
            angle_part = np.angle(t1)*np.sqrt(j) - np.angle(t2)*np.cos(j /2)
            cf[j-1] = mag_part * (np.cos(angle_part) +1j * np.sin(angle_part)) + np.conj(t1)*t2**j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_510(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        r1 = t1.real
        im1 = t1.imag
        r2 = t2.real
        im2 = t2.imag
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(r1 +j) +1) * np.sqrt(j)
            mag_part2 = np.abs(t2)*np.sin(j /3)
            magnitude = mag_part1 + mag_part2
            ang_part1 = np.angle(t1) + np.cos(j * np.pi /5)
            ang_part2 = np.sin(j * np.pi /7) * np.angle(t2)
            angle = ang_part1 + ang_part2
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_511(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            term1 = np.sin(j) * t1.real + np.cos(j *2) * t2.imag
            term2 = np.cos(j /3) * t2.real - np.sin(j /4) * t1.imag
            magnitude = np.log(np.abs(term1 + term2) +1) *j
            angle = np.angle(t1)*np.sin(j /2) + np.angle(t2)*np.cos(j /3) + np.sin(j)**2
            cf[j-1] = magnitude * np.exp(1j * angle) +0.3 * np.exp(1j*(angle /2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_512(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = np.abs(t1)**j + np.abs(t2)**(n -j) + np.log(j + np.abs(t1 - t2))
            angle = np.angle(t1)*j - np.angle(t2)*(n -j) + np.sin(j)*np.cos(j)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        for k in range(1,6):
            cf[k-1] *= np.conj(t1) * np.sin(k)
        for r in range(1,6):
            cf[n -r] *= np.conj(t2) * np.cos(r)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_513(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        real_seq = np.linspace(t1.real, t2.real, n)
        imag_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            magnitude = np.log(np.abs(t1 +j) +1) * np.sin(j * np.pi /7) + np.cos(j * np.pi /5) * np.prod(np.arange(1, (j %5)+2))
            angle = np.angle(t2) + np.sin(j * np.pi /3) * np.cos(j * np.pi /4) + np.tan(j * np.pi /6)
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        for k in range(1, (n //2)+1):
            idx = k *2
            if idx <=n:
                cf[idx-1] *= np.exp(1j * real_seq[k-1]/(np.abs(imag_seq[k-1]) +1))
        for r in range(1, (n%3)+2):
            cf[r-1] = cf[r-1]**2 / (1 + np.abs(cf[r-1]))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_514(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for r in range(1, n+1):
            mag_part = np.log(np.abs(t1) + np.abs(t2) + r) * (np.sin(rec_seq[r-1] * np.pi / (r +1)) + np.cos(imc_seq[r-1] * np.pi / (r +2)))
            angle_part = np.angle(t1)*np.sin(r /5) + np.angle(t2)*np.cos(r /7)
            intricate_sum =0
            for j in range(1,4):
                intricate_sum += (t1.real**j - t2.imag**j) * np.sin(j * rec_seq[r-1]) * np.cos(j * imc_seq[r-1])
            cf[r-1] = mag_part * np.exp(1j * angle_part) + intricate_sum * np.conj(t1)*t2**r
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_515(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            phase = np.sin(j * t1.real + np.cos(j * t2.imag)) + np.cos(j * t2.real - np.sin(j * t1.imag))
            magnitude = np.log(np.abs(t1) +1) + np.sqrt(j)*t2.real - np.abs(t1.imag) + np.prod([t1.real, t2.real]) / (j +1)
            cf[j-1] = magnitude * np.exp(1j * phase)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_516(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec = t1.real
        imc = t2.imag
        for j in range(1, n+1):
            mag_part = np.log(np.abs(t1) +j) * np.sin(j * np.pi /7) + np.sqrt(j)*np.cos(j * np.angle(t2))
            angle_part = np.angle(t1)*j**0.5 + np.angle(t2)*np.log(j +1)
            cf[j-1] = mag_part * np.exp(1j * angle_part) + np.conj(t1)**j * np.cos(j * np.pi /5)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_517(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for r in range(1, n+1):
            mag_part = np.log(np.abs(t1) + np.abs(t2) +r) * np.sin(r * t1.real / (1 +r)) + np.cos(r * t1.imag / (1 +r))
            angle_part = np.angle(t1)*r + np.angle(t2)*(n -r) + np.sin(r * t1.real)*np.cos(r * t2.imag)
            cf[r-1] = mag_part * np.exp(1j * angle_part) + np.conj(mag_part * np.exp(-1j * angle_part))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_518(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, n+1):
            mag = np.log(np.abs(t1) +1) * (j**(np.sin(j) + np.cos(j))) + np.sin(j * np.pi /7) * np.cos(j * np.pi /5)
            angle = np.angle(t1)*np.sin(j /3) + np.angle(t2)*np.cos(j /4) + np.sin(j * np.pi /6)
            for k in range(1, min(j,5)+1):
                mag += r1**k * i2**(j -k) * np.log(k +1)
                angle += np.angle(t1)**k - np.angle(t2)**(j -k) * np.cos(k * np.pi /8)
            cf[j-1] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_519(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag = np.log(np.abs(t1) +j) * np.abs(np.sin(j * np.pi /7)) + np.sqrt(j) * np.cos(j * np.angle(t2))
            angle = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j /3)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_520(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        for j in range(1, n+1):
            magnitude = np.log(np.abs(rec_seq[j-1] + imc_seq[j-1]) +1) * (1 + np.sin(j * np.pi /5)) * (1 + np.cos(j * np.pi /7))
            angle = np.angle(t1)*np.sin(j /3) + np.angle(t2)*np.cos(j /4) + np.sin(j * t1.real)*np.cos(j * t2.imag)
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        for k in range(1, n+1):
            cf[k-1] *= (1 +0.5 * np.conj(cf[max(0, k -2)]) ) +0.3 * np.sin(k)*np.cos(k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_521(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) +j)
            mag_part2 = np.sin(j * np.angle(t2)) + np.cos(j /2 * np.angle(t1))
            magnitude = mag_part1 * (1 + mag_part2**2)
            angle_part1 = np.angle(t1 +j)
            angle_part2 = np.cos(j * t2.imag)
            angle = angle_part1 + angle_part2
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_522(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag =0
            angle =0
            for k in range(1, j+1):
                mag += (t1.real**k * t2.real**(j -k) + t1.imag**k * t2.imag**(j -k))
                angle += (np.angle(t1) * np.sin(k) - np.angle(t2) * np.cos(j -k))
            mag *= np.log(np.abs(t1) + np.abs(t2) +j)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_523(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, n+1):
            mag_part1 = np.log(np.abs(t1) +j) * np.sin(j * np.pi /6)
            mag_part2 = (r2**j + i1**(n -j)) * np.cos(j * np.pi /4)
            mag = mag_part1 + mag_part2
            angle = np.angle(t1)*np.sin(j /3) + np.angle(t2)*np.cos(j /5) + np.sin(j * np.pi /7)
            cf[j-1] = mag * np.exp(1j * angle) + np.conj(t2)*(j %4)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_524(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            phase = np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2)) + np.log(np.abs(t1) + np.abs(t2) + j)
            magnitude = (j**2 + np.sqrt(j)) * np.abs(np.sin(j /3)) + np.exp(-j /10) * np.abs(t1 + t2)
            cf[j-1] = magnitude * (np.cos(phase) +1j * np.sin(phase))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_525(t1, t2):
    try:
        n =40
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part = np.log(np.abs(t1) +1) * (j**np.abs(t2.real)) + np.sum(np.arange(1, j+1)) * np.sqrt(j)
            angle_part = np.angle(t1)*np.sin(j) + np.angle(t2)*np.cos(j) + np.sin(j * t1.imag)*np.cos(j * t2.imag)
            coeff = mag_part * np.exp(1j * angle_part)
            for k in range(1,4):
                coeff += (t1.real**k) * (t2.imag**k) * np.sin(k *j) / (k +1)
            cf[j-1] = coeff + np.conj(t2)*t1.real**((j %5)+1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_526(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            r = t1.real + t2.real *j
            im = t1.imag - t2.imag *j
            mag = np.log(np.abs(r + im*1j) +1) * np.sin(j * np.pi /n) + np.cos(j * np.pi /5)
            angle = np.angle(t1)*np.sin(r * np.pi /7) + np.angle(t2)*np.cos(r * np.pi /4)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_527(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            angle = np.angle(t1)*j + np.angle(t2)/(j +1)
            magnitude = np.abs(t1)**j + np.abs(t2)**(n -j) + np.log(np.abs(t1) + np.abs(t2) +j)
            phase = np.sin(j * t1.real) * np.cos(j * t2.imag) + np.sin(t1.imag * j /2)
            cf[j-1] = magnitude * (np.cos(angle + phase) +1j * np.sin(angle - phase))
        for k in range(1, n//2 +1):
            cf[k-1] *= np.conj(cf[n -k])
        for r in range(1, n+1):
            cf[r-1] += np.exp(1j * (t1.real * r - t2.imag / (r +1)))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_528(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            k = j**1.5
            r = t1.real * np.log(k +1) + t2.real * np.sqrt(k +1)
            im_part = t1.imag * np.sin(k) + t2.imag * np.cos(k)
            magnitude = np.abs(t1)*k + np.abs(t2)/(k +1)
            angle = np.angle(t1)*np.sin(k /10) + np.angle(t2)*np.cos(k /10)
            cf[j-1] = (r +1j * im_part) * (1 + magnitude) * np.exp(1j * angle)
        for k in range(1,6):
            for r in range(1,8):
                index = (k -1)*7 + r
                if index <=n:
                    cf[index-1] *= (1 +0.05 * np.sin(k * r)) +0.02 * np.cos(k +r)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_529(t1, t2):
    try:
        n =35
        cf = np.zeros(n, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, n+1):
            mag = np.log(np.abs(r1 +j)) * np.sin(j * np.pi * i2) + np.sqrt(j) * np.cos(j * np.pi * r2)
            angle = np.angle(t1)*np.log(j +1) + np.angle(t2)*np.sin(j*r1) + np.cos(j *i2)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle)) + np.conj(t1)*np.sin(j) / (j +1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
def poly_530(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=n)
        imc = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(n):
            r = rec[j]
            m = imc[j]
            term1 = np.sin(r * np.pi / (j + 2)) * np.cos(m * np.pi / (j + 3))
            term2 = np.log(np.abs(r + m) + 1) * (t1.real ** (j + 1))
            term3 = np.prod([r, m, j + 1]) ** (1 / (j + 1))
            mag = term1 + term2 + term3
            angle = np.angle(t1) * np.sin(m * np.pi / (j + 4)) + np.angle(t2) * np.cos(r * np.pi / (j + 5)) + np.log(j + 2)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_531(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = t1.real
        imc1 = t1.imag
        rec2 = t2.real
        imc2 = t2.imag

        for r in range(1, n + 1):
            if r % 3 == 1:
                mag = np.log(np.abs(t1 + r) + 1) * np.sin(r / n * np.pi) + np.cos(r * np.pi / 4)
                ang = np.angle(t1) + np.sin(r * np.pi / 6) * np.angle(t2)
            elif r % 3 == 2:
                mag = np.log(np.abs(t2 + r) + 1) * np.cos(r / n * np.pi) + np.sin(r * np.pi / 3)
                ang = np.angle(t2) + np.cos(r * np.pi / 5) * np.angle(t1)
            else:
                mag = np.log(np.abs(t1 * t2 + r) + 1) * np.sin(r / (2 * n) * np.pi) + np.cos(r * np.pi / 2)
                ang = np.angle(t1 * t2) + np.sin(r * np.pi / 4) * np.cos(r * np.pi / 3)
            cf[r - 1] = mag * np.exp(1j * ang)

        for k in range(n):
            if k < n / 3:
                cf[k] = cf[k] * (k + 1)
            elif k < 2 * n / 3:
                cf[k] = cf[k] * (-(k + 1))
            else:
                cf[k] = cf[k] * (1 / (k + 1))
        
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_532(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for r in range(1, n + 1):
            j = r % 7 + 1
            k = np.floor(r / 5) + 1
            magnitude = (np.log(np.abs(t1) + 1) * np.cos(r) + np.log(np.abs(t2) + 1) * np.sin(r)) * (1 + r / 10)
            angle = np.angle(t1) * np.sin(r / 2) - np.angle(t2) * np.cos(r / 3) + np.sin(r) * np.cos(r / 4)
            cf[r - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_533(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            r = t1.real * np.log(k + 1) + t2.real * np.sin(k)
            im = t1.imag * np.cos(k) + t2.imag * np.log(k + 2)
            mag = np.log(np.abs(t1) + k**2) * (1 + np.sin(k / 3))
            ang = np.angle(t1) * np.cos(k / 4) + np.angle(t2) * np.sin(k / 5)
            cf[k - 1] = (r + 1j * im) * mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_534(t1, t2):
    try:
        degree = 35
        cf = np.zeros(degree, dtype=np.complex128)
        for j in range(1, degree + 1):
            r1 = t1.real
            r2 = t2.real
            im1 = t1.imag
            im2 = t2.imag
            mag_part1 = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * r1) + np.cos(j * im2))
            mag_part2 = np.sin(r2 * j**1.3) * np.cos(im1 * np.sqrt(j))
            magnitude = mag_part1 * mag_part2 + np.log(j + 1)
            angle = np.angle(t1) * np.sin(j / 2) + np.angle(t2) * np.cos(j / 3)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_535(t1, t2):
    try:
        n = ps.poly.get("n") or 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=n)
        imc = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(1, n + 1):
            mag = np.log(np.abs(rec[j - 1] + imc[j - 1]*1j) + 1) * np.sin(j * np.pi / 5) + np.cos(j * np.pi / (j + 2))
            ang = np.angle(rec[j - 1] + imc[j - 1]*1j) + np.sin(j / n * np.pi * 4) - np.cos(j / n * np.pi * 3)
            cf[j - 1] = mag * (np.cos(ang) + np.sin(ang) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_536(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = t1.real * np.log(j + 1) + t2.real * np.sin(j / 3)
            im = t1.imag * np.cos(j / 4) - t2.imag * np.log(j + 2)
            magnitude = np.log(np.abs(t1) + j**1.2) * (1 + 0.5 * np.sin(j * np.pi / 6))
            angle = np.angle(t1) * np.cos(j / 5) + np.angle(t2) * np.sin(j / 7) + np.log(np.abs(t2) + 1)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j) + (r + im) * 1j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_537(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, num=n)
        imc_seq = np.linspace(t1.imag, t2.imag, num=n)
        for k in range(1, n + 1):
            magnitude = np.log(np.abs(rec_seq[k - 1] + imc_seq[k - 1]) + 1) * (np.sin(k * np.pi / 7) + np.cos(k * np.pi / 5))
            angle = np.angle(t1 * t2) + np.sin(k) - np.cos(k / 2)
            cf[k - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_538(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=n)
        imc = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(1, n + 1):
            magnitude = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.pi / 7)) + np.sqrt(j) * np.cos(j * np.pi / 5)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j / 3) + np.sin(j * imc[j - 1]) - np.cos(j * rec[j - 1])
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_539(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            k = j**2 + int(np.floor(t1.real))
            r = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.angle(t2)))
            s = np.cos(j * t2.real) * np.sin(j * t1.imag) + np.cos(j * t2.imag)
            magnitude = r + np.log(np.abs(t2) + j)
            angle = s + np.sin(j * t1.real) * np.cos(np.angle(t1))
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_540(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, 36):
            r1 = t1.real
            i1 = t1.imag
            r2 = t2.real
            i2 = t2.imag

            if j <= 10:
                mag = np.log(np.abs(t1) + 1) * np.sin(j * r2) + (np.abs(t2)**j) / (j + 1)
                angle = np.angle(t1) + j * np.angle(t2)
            elif j <= 25:
                mag = np.cos(j * i1) * np.log(np.abs(t2) + 1) + np.real(np.conj(t1)) / (j + 2)
                angle = np.angle(t1) * np.sin(j * np.pi / 5) + np.angle(t2) * np.cos(j * np.pi / 7)
            else:
                mag = np.sin(j * r1 + np.cos(j * i2)) * np.log(np.abs(t1) + np.abs(t2) + 1)
                angle = (np.angle(t1)**2) / (j + 3) + (np.angle(t2)**2) / (j + 4)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_541(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(0.3 * j * t2.real)
            mag_part2 = np.log(np.abs(t2) + j) * np.cos(0.2 * j * t1.imag)
            mag = mag_part1 + mag_part2
            angle_part1 = np.angle(t1) + j * 0.1 * np.pi * np.sin(j / 5)
            angle_part2 = np.angle(t2) + j * 0.1 * np.pi * np.cos(j / 3)
            angle = angle_part1 + angle_part2
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_542(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=n)
        imc = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(1, n + 1):
            mag_sum = 0
            for k in range(1, j + 1):
                mag_sum += np.sin(k * t1.real) * np.cos(k * t2.imag)
            magnitude = np.log(np.abs(rec[j - 1] + imc[j - 1]) + 1) * mag_sum
            angle = 0
            for r in range(1, j + 1):
                angle += np.angle(t1) * np.sin(r) + np.angle(t2) * np.cos(r)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_543(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part1 = np.log(np.abs(t1) * j + 1)
            mag_part2 = np.log(np.abs(t2) + np.sqrt(j))
            mag_variation = mag_part1 * np.sin(j * t1.real) + mag_part2 * np.cos(j * t2.imag)
            angle_part1 = np.angle(t1) * j**1.3
            angle_part2 = np.angle(t2) / (j + 1)
            angle_variation = angle_part1 - angle_part2 + np.sin(j) * np.cos(j / 2)
            cf[j - 1] = (np.abs(mag_variation) + 1) * np.exp(1j * angle_variation)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_544(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part1 = np.log(np.abs(t1) + j) * np.sqrt(np.abs(t2) + j)
            mag_part2 = np.sin(j * t1.real) * np.cos(j * t2.imag)
            magnitude = mag_part1 + mag_part2 * (j % 5 + 1)
            
            angle_part1 = np.angle(t1) * np.sin(j / 3)
            angle_part2 = np.angle(t2) * np.cos(j / 4)
            angle_part3 = np.log(np.abs(t1) + np.abs(t2) + j)
            angle = angle_part1 + angle_part2 + angle_part3
            
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        for k in range(1, n + 1):
            cf[k - 1] = cf[k - 1] * np.conj(t1) / (np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_545(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=n)
        imc = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(1, n + 1):
            mag = (rec[j - 1]**3 + imc[j - 1]**2) * np.log(np.abs(t1) + 1) + np.sin(j * np.pi / 4) * np.cos(j * np.pi / 3)
            ang = np.angle(t1) + np.angle(t2) * j + np.sin(j * t1.real * t2.imag)
            cf[j - 1] = mag * np.exp(1j * ang) + np.conj(t1)**(j % 5) * np.cos(j * imc[j - 1])
        for k in range(1, n + 1):
            cf[k - 1] += (np.prod(rec[:k] + imc[:k])) * np.sin(k * np.angle(t2))
        for r in range(1, n + 1):
            cf[r - 1] = cf[r - 1] * (1 + np.abs(t1 - t2) / (r + 1)) + np.log(np.abs(t1 + t2) + 1) * np.cos(r * np.angle(t1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_546(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = t1.real
        imc1 = t1.imag
        rec2 = t2.real
        imc2 = t2.imag
        for j in range(1, n + 1):
            r = (rec1 + rec2) / 2 + np.sin(j * np.pi / 7) * np.cos(j * np.pi / 5)
            theta = (imc1 - imc2) / n * j + np.sin(j * np.pi / 3)
            magnitude = np.log(np.abs(t1) * j + np.abs(t2) * (n - j + 1)) + np.sqrt(j)
            cf[j - 1] = magnitude * (np.cos(theta) + 1j * np.sin(theta))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_547(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag = np.log(np.abs(t1) + j) * np.sin(j * t1.real) + np.log(np.abs(t2) + j) * np.cos(j * t1.imag)
            angle = np.angle(t1) * j**2 - np.angle(t2) * np.sqrt(j)
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        for k in range(1, n + 1):
            cf[k - 1] *= np.prod((k + t1.real) / (1 + k * t2.real)) + np.sum([np.cos(k * np.angle(t1)), np.sin(k * np.angle(t2))])
        for r in range(1, n + 1):
            cf[r - 1] += np.conj(cf[n - r]) * np.abs(t1)**(1/r)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_548(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = t1.real * np.log(j + 1) + t2.real * np.sqrt(j)
            im = t1.imag * np.sin(j) + t2.imag * np.cos(j * np.pi / 4)
            mag = np.abs(t1)**(j % 5 + 1) + np.abs(t2)**(n - j + 1)
            angle = np.angle(t1) * j + np.angle(t2) / (j + 1)
            cf[j - 1] = (mag * np.exp(1j * angle)) + np.conj(t1) * np.conj(t2) / (j + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_549(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag = np.log(np.abs(t1) * j + 1) + np.sin(j * t2.real)**2 + np.cos(j * t1.imag)
            angle = np.angle(t1) * j + np.angle(t2) / (j + 1) + np.sin(j / 3)
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_550(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=n)
        imc = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(1, n + 1):
            coeff = 0
            for k in range(1, j + 1):
                coeff += (rec[k - 1]**2 - imc[k - 1]**3) * np.exp(1j * np.sin(k * np.pi / n))
                coeff += np.conj(t1) * np.cos(k * np.pi / (j + 1))
            for r in range(1, n - j + 1):
                coeff += np.log(np.abs(rec[j - 1] + imc[j - 1]) + 1) * r * np.sin(r * np.pi / n)
                coeff += np.abs(t2)**r * np.cos(r * np.angle(t1 + t2))
            cf[j - 1] = coeff
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_551(t1, t2):
    try:
        n = 40
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            rec_t1 = t1.real
            imc_t1 = t1.imag
            rec_t2 = t2.real
            imc_t2 = t2.imag

            mag_part1 = np.log(np.abs(t1) + 1) * np.sin(j * np.pi / n)
            mag_part2 = np.log(np.abs(t2) + 1) * np.cos(j * np.pi / (n / 2))
            mag_variation = mag_part1 + mag_part2 + np.prod([rec_t1, imc_t2])**(1 / j)

            angle_part1 = np.angle(t1) * np.sin(j / 2)
            angle_part2 = np.angle(t2) * np.cos(j / 3)
            angle_variation = angle_part1 + angle_part2 + np.sin(j) * np.cos(j)

            complex_component = np.cos(angle_variation) + 1j * np.sin(angle_variation)

            cf[j - 1] = mag_variation * complex_component + np.conj(t1) * np.sin(j) + np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_552(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.pi / 7) + np.log(np.abs(t2) + 1) * np.cos(j * np.pi / 11)
            angle_part = np.angle(t1) * np.sin(j / 2) + np.angle(t2) * np.cos(j / 3)
            intricate_sum = 0
            for k in range(1, j + 1):
                intricate_sum += (t1.real**k - t2.imag**k) * np.sin(k * np.pi / (j + 1))
            cf[j - 1] = mag_part * np.exp(1j * angle_part) + np.conj(t1) * (t2**(j % 5)) + intricate_sum
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_553(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_sum = 0
            ang_sum = 0
            for k in range(1, j + 1):
                term_mag = np.log(np.abs(t1) + k**2) * np.sin(k * t2.real) + np.cos(k * t1.imag)
                term_ang = np.angle(t2) * np.sqrt(k) + np.sin(k / 2)
                mag_sum += term_mag
                ang_sum += term_ang
            cf[j - 1] = (mag_sum * np.exp(1j * ang_sum)) + np.conj(t1) * (t2**j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_554(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec1 = t1.real
        rec2 = t2.real
        imc1 = t1.imag
        imc2 = t2.imag
        for j in range(1, n + 1):
            term1 = rec1**j * np.sin(j * np.pi / 4)
            term2 = imc2 * np.cos(j * np.pi / 3)
            term3 = np.log(np.abs(t1) + 1) * t2.real**((j % 5) + 1)
            term4 = np.abs(t1 + t2)**(0.5 * j)
            angle = np.angle(t1) + np.angle(t2) * j
            magnitude = np.abs(term1 + term2 + term3) + term4
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        for k in range(1, n + 1):
            cf[k - 1] = cf[k - 1] * np.exp(1j * np.sin(k)) + np.conj(cf[(k % n)])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_555(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            real_part = t1.real**j + t2.real**((j % 3) + 1)
            imag_part = t1.imag**((j % 4) + 1) + t2.imag**((j % 5) + 1)
            magnitude = real_part + imag_part + np.log(np.abs(t1 * t2) + 1)
            angle = np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2))
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle)*1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_556(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=n)
        imc = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(1, n + 1):
            angle = np.sin(j * rec[j - 1]) + np.cos(j * imc[j - 1]) + np.angle(t1 + t2)
            magnitude = np.log(np.abs(rec[j - 1]**2 + imc[j - 1]**2) + 1) * (j**1.5 + np.prod(rec[:j] + imc[:j]))
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_557(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n + 1):
            mag_sum = 0
            for j in range(1, k + 1):
                mag_sum += np.sin(j * np.pi / 7) * np.cos(j * np.pi / 5) + np.log(np.abs(t1 * j + t2 / (j + 1)) + 1)
            magnitude = mag_sum * (1 + np.abs(t1 - t2) / 10)
            
            angle_sum = 0
            for j in range(1, k + 1):
                angle_sum += np.angle(t1 + j * t2) * np.sin(j * np.pi / 9) - np.angle(t2) * np.cos(j * np.pi / 11)
            angle = angle_sum / k
            
            cf[k - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_558(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = t1.real * np.log(j + 1) + t2.real * np.sqrt(j)
            theta = np.sin(j * t1.imag) + np.cos(j * t2.imag) + np.angle(t1 + t2)
            for k in range(1, 4):
                r += t1.real * k / (j + 1)
                theta += np.sin(k * np.pi / j)
            cf[j - 1] = r * (np.cos(theta) + 1j * np.sin(theta))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_559(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            temp_real = 0
            temp_imag = 0
            for k in range(1, j + 1):
                temp_real += (t1.real**k) * np.sin(k * t2.real) / (k + 1)
                temp_imag += (t2.imag**(j - k + 1)) * np.cos((j - k + 1) * t1.imag) / (j - k + 2)
            magnitude = np.log(np.abs(temp_real + temp_imag) + 1) * j
            angle = t1.real * t2.imag / j + np.sin(j * np.angle(t1 + t2))
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_560(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        re1 = t1.real
        im1 = t1.imag
        re2 = t2.real
        im2 = t2.imag
        for j in range(1, n + 1):
            magnitude = np.log(np.abs(t1) + j) * np.sin(j * np.pi / n) + np.log(np.abs(t2) + np.sqrt(j)) * np.cos(j * np.pi / (n + 1))
            angle = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 5)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_561(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = t1.real * np.log(j + 1) + t2.imag * np.sin(j * t1.real)
            theta = np.angle(t1) * np.cos(j) - np.angle(t2) * np.sin(j)
            mag_variation = np.abs(t1)**j / (1 + j) + np.abs(t2)**(np.sqrt(j))
            cf[j - 1] = (r + 1j * theta) * (mag_variation + np.sin(j) - np.cos(j))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_562(t1, t2):
    try:
        n = 35
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            rec = t1.real + (t2.real - t1.real) * j / n
            imc = t1.imag + (t2.imag - t1.imag) * j / n
            mag = np.log(np.abs(rec) + 1) * (j**2 + np.sqrt(n - j + 1)) * np.sin(j) + np.prod(np.arange(1, (j % 5) + 2))
            angle = np.sin(rec * np.pi / 7) + np.cos(imc * np.pi / 5) + np.angle(t1) - np.angle(t2)
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_563(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        for j in range(1, 27):
            mag_part = np.log(np.abs(t1) + j) * (j**2 + t2.real * np.sin(j * np.pi / 5))
            angle_part = np.angle(t1) * np.cos(j * np.pi / 7) - np.angle(t2) * np.sin(j * np.pi / 3)
            cf[j - 1] = mag_part * (np.cos(angle_part) + np.sin(angle_part) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_564(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        for j in range(1, 27):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(j * t2.real)
            mag_part2 = np.cos(j * t1.imag) * np.abs(t2 + j)
            mag = mag_part1 + mag_part2
            
            angle_part1 = np.angle(t1) * j**0.5
            angle_part2 = np.sin(j * np.pi / 7) + np.cos(j * np.pi / 11)
            angle = angle_part1 + angle_part2
            
            cf[j - 1] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_565(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        for j in range(1, 27):
            a = t1.real + np.sin(j) * t2.real
            b = t1.imag + np.cos(j) * t2.imag
            c = np.log(np.abs(t1) + np.abs(t2) + 1)
            d = np.angle(t1) * j + np.angle(t2) / (j + 1)
            cf[j - 1] = (a + 1j * b) * c * (np.cos(d) + 1j * np.sin(d)) + np.conj(t1)**j * np.cos(j) - np.conj(t2) * np.sin(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_566(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree, dtype=np.complex128)
        for j in range(1, degree + 1):
            mag_part1 = np.log(np.abs(t1) + j)
            mag_part2 = np.prod(np.arange(1, (j % 5) + 2))
            magnitude = mag_part1 * (mag_part2 + np.sqrt(j))
            
            angle_part1 = np.sin(j * np.angle(t1))
            angle_part2 = np.cos(np.angle(t2) / (j + 1))
            angle = angle_part1 + angle_part2
            
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
            
            # Introduce variation based on imaginary components
            cf[j - 1] += (t1.imag - t2.imag) * np.sin(j)**2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_567(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for j in range(1, 26):
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * t1.real) + np.cos(j * t2.imag) + t1.real**0.5 * t2.imag**0.3)
            angle = np.angle(t1) * j + np.angle(t2) * np.sin(j / 3) + np.cos(j / 5)
            cf[j - 1] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_568(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=degree + 1)
        imc = np.linspace(t1.imag, t2.imag, num=degree + 1)
        for r in range(1, degree + 2):
            mag = np.log(np.abs(t1 * rec[r - 1] + t2 * imc[r - 1]) + 1) * (1 + np.sin(r * np.pi / 4)) + np.cos(r * np.pi / 5)
            ang = np.angle(t1) * rec[r - 1] + np.angle(t2) * imc[r - 1] + np.sin(r * np.pi / 6)
            cf[r - 1] = mag * (np.cos(ang) + np.sin(ang) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_569(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=degree + 1)
        imc = np.linspace(t1.imag, t2.imag, num=degree + 1)
        for j in range(1, degree + 2):
            mag_part = np.log(np.abs(t1) + j) + np.sin(rec[j - 1] * np.pi / (j + 1)) * np.cos(imc[j - 1] * np.pi / (j + 2))
            angle_part = np.angle(t1) * j + np.sin(rec[j - 1] / (j + 1)) - np.cos(imc[j - 1] / (j + 2))
            cf[j - 1] = mag_part * (np.cos(angle_part) + np.sin(angle_part) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_570(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            sum_mag = 0
            sum_ang = 0
            for k in range(1, j + 1):
                term = t1 * k + t2 / k
                sum_mag += np.log(np.abs(term) + 1)
                angle_term = t1 * np.sin(k * t2.real) + t2 * np.cos(k * t1.imag)
                sum_ang += np.angle(angle_term)
            cf[j - 1] = sum_mag * (np.cos(sum_ang) + np.sin(sum_ang) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_571(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            k = j % 7 + 1
            r = t1.real * np.sin(k * t2.real) + t2.real * np.cos(k * t1.imag)
            i_part = t1.imag * np.cos(k * t2.real) - t2.imag * np.sin(k * t1.imag)
            magnitude = np.log(np.abs(t1) + j) * (j**1.5) / (1 + np.log(j + 1))
            angle = np.angle(t1) * j + np.log(j + 1) * np.angle(t2)
            cf[j - 1] = magnitude * np.exp(1j * angle) * (r + 1j * i_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_572(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for j in range(1, 26):
            summation_mag = 0
            summation_ang = 0
            for k in range(1, j + 1):
                summation_mag += np.log(np.abs(t1 * k + t2) + 1) * np.sin(k * t1.real)
                summation_ang += np.angle(t1 * k - t2) + np.cos(k * t2.imag)
            cf[j - 1] = summation_mag * (np.cos(summation_ang) + np.sin(summation_ang) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_573(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            part1 = t1.real * np.sin(j * np.angle(t2)) + t2.real * np.cos(j * np.angle(t1))
            part2 = t1.imag * np.cos(j * t2.real) - t2.imag * np.sin(j * t1.real)
            magnitude = np.log(np.abs(t1) + j) + np.sum(np.sin(np.arange(1, j + 1) * t1.real) * np.cos(np.arange(1, j + 1) * t2.imag))
            angle = np.angle(t1) * j + np.angle(t2) * j**0.5
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_574(t1, t2):
    try:
        deg = 25
        cf = np.zeros(deg + 1, dtype=np.complex128)
        for j in range(1, deg + 2):
            r_part = t1.real**j * np.cos(j * np.angle(t2)) + t2.real**(deg + 1 - j) * np.sin(j * np.angle(t1))
            im_part = t1.imag**j * np.sin(j * np.angle(t2)) - t2.imag**(deg + 1 - j) * np.cos(j * np.angle(t1))
            magnitude = np.log(np.abs(r_part + im_part) + 1) * (j**1.5)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        for k in range(1, deg + 2):
            factor = (k % 3) + 1
            cf[k - 1] *= (t1.real**(k % 5) + t2.imag**(k % 4)) * np.sin(k * np.angle(cf[k - 1]))
        for r in range(2, deg):
            cf[r - 1] += (cf[r - 2] * cf[r]) / (1 + np.abs(cf[r - 1]))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_575(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            k = (j * 3 + 7) % 10
            r = t1.real * np.cos(j) + t2.real * np.sin(k)
            im = t1.imag * np.sin(j) + t2.imag * np.cos(k)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + j % 5) / (j + 1)
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(j)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_576(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            r = j / degree
            k = (j**2 + 3*j + 1)
            mag = np.log(np.abs(t1) + np.abs(t2) + r * k) * (1 + np.sin(j) * np.cos(k))
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(r * j)
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_577(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, num=degree + 1)
        imc_seq = np.linspace(t1.imag, t2.imag, num=degree + 1)
        for j in range(1, degree + 2):
            r = rec_seq[j - 1]
            im = imc_seq[j - 1]
            mag = np.log(np.abs(r) + np.abs(im) + 1) * np.sin(2 * np.pi * r) + np.cos(3 * np.pi * im)
            ang = np.angle(t1) * j + np.sin(im * np.pi)
            cf[j - 1] = mag * (np.cos(ang) + np.sin(ang) * 1j) + np.conj(t2) * np.cos(j * np.pi / degree)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_578(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for j in range(1, 26):
            r = t1.real * j + t2.real / (j + 1)
            im = t1.imag * np.sin(j) + t2.imag * np.cos(j / 2)
            magnitude = np.log(np.abs(t1) + j) + np.sin(j * t2.real) * np.cos(j * t1.imag)
            angle = np.angle(t1) * j - np.angle(t2) / (j + 0.5)
            cf[j - 1] = magnitude * np.exp(1j * angle) + np.conj(t1) * t2**(j % 7)
        for k in range(1, 26):
            cf[k - 1] = cf[k - 1] * (1 + 0.05 * np.sin(k * t1.real)) + 0.05j * np.cos(k * t2.imag)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_579(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            mag_part = np.abs(t1)**j * np.log(np.abs(t2) + 1) + np.abs(t2)**(degree +1 -j) * np.sin(j)
            angle_part = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j - 1] = mag_part * np.exp(1j * angle_part)
        for k in range(1, degree + 2):
            cf[k - 1] += np.conj(t1) * t2.real / (k + 1)
            cf[k - 1] = cf[k - 1] * (1 + np.sin(k * np.pi / 12)) + np.cos(k * np.pi / 18) * t1.imag
        for r in range(1, degree + 2):
            cf[r - 1] += np.prod([np.abs(t1), np.abs(t2)]) / (r + 2)
            cf[r - 1] = cf[r - 1] * np.log(np.abs(cf[r - 1]) + 1) + np.exp(1j * np.sin(r))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_580(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(2 * np.pi * t1.real / (j + 1))
            mag_part2 = np.log(np.abs(t2) + j) * np.cos(2 * np.pi * t2.imag / (j + 1))
            magnitude = mag_part1 + mag_part2 + np.prod([t1.real, t1.imag, j])
            angle = np.angle(t1) * j + np.angle(t2) * (degree +1 - j) + np.sin(j) - np.cos(j)
            cf[j - 1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_581(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            term1 = t1.real * np.sin(j * np.angle(t2)) + t2.real * np.cos(j * t1.imag)
            term2 = t1.imag * np.cos(j * t2.real) - t2.imag * np.sin(j * np.angle(t1))
            magnitude = np.log(np.abs(t1) + j) + np.abs(t2)**j
            angle = term1 + term2
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j) + np.conj(t1) * np.conj(t2) / (j + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_582(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        for j in range(1, 27):
            r = t1.real**(j % 5 + 1) + t2.real**(j % 7 + 1)
            imc = t1.imag**(j % 3 + 2) - t2.imag**(j % 4 + 1)
            magnitude = np.log(np.abs(t1) + j) * np.sin(r) + np.cos(imc)
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(imc)
            cf[j - 1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_583(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag = np.log(np.abs(t1.real * j**2 + t2.imag / (j + 1)) + 1) * (1 + np.sin(j * np.pi / 4)) * (1 + np.cos(j * np.pi / 5))
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j**2) + np.log(j + 1)
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_584(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = t1.real * np.sin(j) + t2.real * np.cos(j)
            k = t1.imag * np.cos(j) - t2.imag * np.sin(j)
            magnitude = np.log(np.abs(r) + 1) * (1 + (j % 5)) + np.abs(k)**1.5
            angle = np.angle(t1) * j + np.angle(t2) * np.sqrt(j)
            cf[j - 1] = magnitude * np.exp(1j * angle) + np.conj(t1) * np.conj(t2) / (j + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_585(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            k = j**2
            r = t1.real * np.sin(k * np.pi / 7) + t2.real * np.cos(k * np.pi / 5)
            s = t1.imag * np.cos(k * np.pi / 3) - t2.imag * np.sin(k * np.pi / 4)
            mag = np.log(np.abs(t1) + np.abs(t2) + k) * (np.abs(r) + np.abs(s) + 1)
            angle = np.angle(t1) * np.log(k + 1) + np.sin(r) - np.cos(s)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_586(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(j * np.pi / 7)
            mag_part2 = np.log(np.abs(t2) + j) * np.cos(j * np.pi / 5)
            magnitude = mag_part1 + mag_part2 + j
            angle_part1 = np.angle(t1) * np.cos(j * np.pi / 4)
            angle_part2 = np.angle(t2) * np.sin(j * np.pi / 3)
            angle = angle_part1 + angle_part2
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, n + 1):
            cf[k - 1] = cf[k - 1] * (1 + 0.05 * k**2) * np.exp(-k / n)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_587(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        n = 25
        for j in range(1, n + 1):
            r1 = t1.real + j * t2.real
            i1 = t1.imag - j * t2.imag
            magnitude = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t2)) + np.cos(j * np.pi / 12)
            angle = np.angle(t1) * np.cos(j) + np.sin(j * np.angle(t2))
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_588(t1, t2):
    try:
        degree = 25
        cf = np.zeros(35, dtype=np.complex128)
        for j in range(1, degree + 1):
            mag = np.log(np.abs(t1) + j**1.3) * np.abs(np.sin(j * np.pi / 4)) + np.abs(t2) * np.cos(j * np.pi / 6)
            angle = np.angle(t1) * np.sin(j / 3) + np.angle(t2) * np.cos(j / 5) + np.sin(j * np.pi / 7)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(degree + 1, 36):
            cf[k - 1] = np.log(k + 1) * (np.sin(k * np.angle(t1)) + 1j * np.cos(k / 2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_589(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for j in range(1, 26):
            k = j + 1
            r = t1.real * np.log(np.abs(t2) + 1) / (j + 1)
            theta = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j * np.pi / 8)
            mag = (np.abs(t1)**j + np.abs(t2)**(25 - j)) * (1 + np.sin(j * np.pi / 5))
            cf[j - 1] = mag * np.exp(1j * theta) + np.conj(t1) * np.cos(j * np.pi / 7)
        for k in range(1, 26):
            cf[k - 1] += (1 + 0.05 * np.sin(k * t1.real)) + 0.05j * np.cos(k * t2.imag)
        for r in range(1, 26):
            cf[r - 1] += np.prod([np.abs(t1), np.abs(t2)]) / (r + 2)
            cf[r - 1] = cf[r - 1] * np.log(np.abs(cf[r - 1]) + 1) + np.exp(1j * np.sin(r))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_590(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            r_part = t1.real * j + t2.real / (j +1)
            i_part = t1.imag * np.sin(j) + t2.imag * np.cos(j)
            magnitude = np.log(np.abs(r_part) + 1) + np.abs(t1) * np.abs(t2) / (j +1)
            angle = np.angle(t1) * j - np.angle(t2) / (j +1) + np.sin(j * np.pi / 5)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        for k in range(1, degree + 2):
            cf[k - 1] *= (1 + 0.05 * k**2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_591(t1, t2):
    try:
        n = 25
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            k = j * 3 + t1.real - t2.imag
            r = np.log(np.abs(t1) + np.abs(t2) + j) * (np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2)))
            angle = np.sin(k * np.angle(t1)) + np.cos(k * np.angle(t2))
            cf[j - 1] = r * (np.cos(angle) + np.sin(angle) * 1j) + np.conj(t1)**k * np.sin(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_592(t1, t2):
    try:
        n = 25
        cf = np.zeros(n + 1, dtype=np.complex128)
        for j in range(1, n + 2):
            sum_val = 0
            prod_val = 1
            for k in range(1, j + 1):
                sum_val += np.sin(k * t1.real) * np.cos(k * t2.imag)
                prod_val *= (t1.real + t2.imag * k)
            magnitude = np.log(np.abs(t1) + j) * sum_val + prod_val
            angle = np.angle(t1) * j - np.angle(t2) * j**2
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j) + np.conj(t1) * np.conj(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_593(t1, t2):
    try:
        deg = 25
        cf = np.zeros(deg + 1, dtype=np.complex128)
        for j in range(1, deg + 2):
            mag = np.log(np.abs(t1) + j) * (1 + np.sin(j * t2.real)) * (1 + np.cos(j * t1.imag))
            ang = np.angle(t1) * j + np.angle(t2) * np.sqrt(j)
            cf[j - 1] = mag * complex(np.cos(ang), np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_594(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            r = t1.real * (j**2 + np.sin(j)) + t2.real * np.log(j + 1)
            im = t1.imag * (np.cos(j / 2) + j) + t2.imag * np.sin(j)
            mag = np.abs(r + im * 1j) * (1 + np.cos(j * np.pi / 5))
            angle = np.angle(r + im * 1j)
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_595(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            magnitude = np.log(np.abs(t1) + j) * np.sqrt(j) * (1 + np.sin(j)) + np.abs(t2) / (j + 1)
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j) + np.sin(j * np.pi / 3)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j) + np.conj(t1) * np.cos(j / n)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_596(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        n = 25
        for j in range(1, n + 1):
            k = (j * 3 + 7) % 10
            r = t1.real * np.sin(j) + t2.imag * np.cos(k)
            mag = np.log(np.abs(t1) + j**2) * np.sin(k * np.pi / 4) + np.cos(r)
            angle = np.angle(t1) * np.cos(j) + np.sin(k * np.angle(t2))
            cf[j - 1] = mag * (np.cos(angle) + np.sin(angle) * 1j) + np.conj(t2)**k
        cf[25] = np.sum(np.abs(cf[:n]) * np.cos(np.arange(1, n + 1) * np.pi / 6)) + np.prod([np.abs(t1), np.abs(t2)])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_597(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag

        for j in range(0, degree + 1):
            mag = 0
            angle = 0
            for k in range(1, j + 2):
                term_mag = np.log(np.abs(t1) * k + 1) * np.sin(k * np.pi * r1) + np.cos(k * np.pi * i2)
                term_angle = np.angle(t1) * k**2 - np.angle(t2) * np.sqrt(k)
                mag += term_mag * np.exp(1j * term_angle)
            if j < degree / 3:
                mag *= (j + 1)
            elif j < 2 * degree / 3:
                mag /= (j + 1)
            else:
                mag *= (j + 1)**2
            cf[j] = mag
        cf[0] = (t1.real * t2.real) + 1j * (t1.imag - t2.imag) + np.sin(t1.real) * np.cos(t2.imag)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_598(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            rec1 = t1.real * np.log(j + 1)
            rec2 = t2.real * np.sin(j * np.pi / 7)
            imc1 = t1.imag * np.cos(j * np.pi / 5)
            imc2 = t2.imag * np.sin(j * np.pi / 3)
            mag = np.log(np.abs(t1) + j) * (1 + (j % 2)) + np.abs(t2)**(j / 2)
            ang = np.angle(t1) * np.sin(j / 4) - np.angle(t2) * np.cos(j / 6)
            cf[j - 1] = (rec1 + rec2) + 1j * (imc1 + imc2) + mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_599(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            part1 = np.sin(j * t1.real) * np.cos(j * t2.imag)
            part2 = np.log(np.abs(t1) + j) + np.log(np.abs(t2) + j)
            part3 = t1.real**j - t2.imag**j
            angle = np.angle(t1) * j + t2.real * np.sin(j)
            cf[j - 1] = (part1 * part2 + part3) * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_600(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        rec_step = np.linspace(t1.real, t2.real, num=degree + 1)
        imc_step = np.linspace(t1.imag, t2.imag, num=degree + 1)
        for j in range(1, degree + 2):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(j * np.pi / 12)
            mag_part2 = np.cos(j * np.pi / 8) * np.log(np.abs(rec_step[j - 1] - imc_step[j - 1]) + 1)
            magnitude = mag_part1 + mag_part2 + j**0.8
            
            angle_part1 = np.angle(t1) * np.sin(j * np.pi / 10)
            angle_part2 = np.angle(t2) * np.cos(j * np.pi / 14)
            angle_part3 = np.sin(j * np.pi / 6) - np.cos(j * np.pi / 9)
            angle = angle_part1 + angle_part2 + angle_part3
            
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_601(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            mag = np.log(1 + np.abs(t1) * j) * (1 + np.sin(j * np.pi / 6)) + np.cos(j * np.pi / 7)
            angle = np.angle(t1) * np.cos(j / 3) + np.angle(t2) * np.sin(j / 4) + np.log(j + 1)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t2)**(j % 5)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_602(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=degree + 1)
        imc = np.linspace(t1.imag, t2.imag, num=degree + 1)
        for j in range(1, degree + 2):
            mag_sum = 0
            angle_sum = 0
            for k in range(1, j + 1):
                mag_sum += np.log(np.abs(rec[k - 1] + imc[k - 1]) + 1) * np.sin(k * np.pi / 4)
                angle_sum += np.angle(rec[k - 1] + imc[k - 1] * 1j) * np.cos(k * np.pi / 3)
            magnitude = mag_sum * np.cos(j * np.pi / 5) + np.abs(t1) / (j + 1)
            angle = angle_sum + np.sin(j * np.angle(t2))
            cf[j - 1] = magnitude * np.exp(1j * angle) + np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_603(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        for j in range(1, 27):
            phase = np.sin(j * t1.real) + np.cos(j * t2.imag)
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (j**1.5) * ((j % 5) + 1)
            real_part = t1.real * np.cos(phase) - t2.imag * np.sin(phase)
            imag_part = t2.real * np.sin(phase) + t1.imag * np.cos(phase)
            cf[j - 1] = (real_part + 1j * imag_part) * mag
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_604(t1, t2):
    try:
        n = 25
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=n)
        imc = np.linspace(t1.imag, t2.imag, num=n)
        for j in range(1, n + 1):
            magnitude = np.log(np.abs(rec[j - 1]) + 1) * (np.abs(t1)**(j % 5 + 1)) + np.abs(t2)**((n - j) % 7 + 1)
            angle = np.sin(rec[j - 1] * np.pi * j) + np.cos(imc[j - 1] * np.pi / (j + 1)) + np.angle(t1) * np.log(j + 2) - np.angle(t2) * np.sqrt(j)
            cf[j - 1] = magnitude * np.exp(1j * angle)
        # Introduce variation using product and sum
        cf = cf * (np.prod(rec) / (np.sum(imc) + 1)) + np.sum(rec) * np.conj(t1) - np.sum(imc) * np.conj(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_605(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(2 * np.pi * t1.real / (j + 1))
            mag_part2 = np.log(np.abs(t2) + j) * np.cos(2 * np.pi * t2.imag / (j + 1))
            magnitude = mag_part1 + mag_part2 + np.prod([t1.real, t1.imag, j])
            angle = np.angle(t1) * j + np.angle(t2) * (degree + 1 - j) + np.sin(j) - np.cos(j)
            cf[j - 1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_606(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, degree + 2):
            r_part = r1 * np.sin(j * np.pi / 7) + r2 * np.log(np.abs(j) + 1)
            i_part = i1 * np.cos(j * np.pi / 5) - i2 * np.sin(j * np.pi / 3)
            magnitude = np.log(np.abs(t1) + j**2) * (1 + np.sin(j * np.pi / 4)) + np.cos(j * np.pi / 6)
            angle = np.angle(t1) * j + np.angle(t2) * (degree + 1 - j) + np.sin(j * np.pi / 8)
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_607(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            real_part = t1.real**j * np.log(np.abs(t2) + j) + np.cos(j * np.angle(t1 + t2))
            imag_part = np.sin(j * np.angle(t1)) * np.abs(t2)**j + (t1.real + t2.real) / (j + 1)
            cf[j - 1] = real_part + 1j * imag_part
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_608(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=degree + 1)
        imc = np.linspace(t1.imag, t2.imag, num=degree + 1)
        for j in range(1, degree + 2):
            mag = np.log(np.abs(rec[j - 1]) + j) * np.sin(2 * np.pi * imc[j - 1]) + np.cos(3 * np.pi * rec[j - 1])
            ang = np.angle(t1) * j + np.sin(np.pi * imc[j - 1]) - np.cos(np.pi * rec[j - 1])
            cf[j - 1] = mag * (np.cos(ang) + np.sin(ang) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_609(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            rec_part = t1.real * np.sin(j * t2.real) + np.cos(j * np.angle(t1))
            imc_part = t2.imag * np.cos(j * t1.imag) + np.sin(j * np.angle(t2))
            mag = np.log(np.abs(t1) + j) * rec_part + np.abs(t2)**0.5 * imc_part
            angle = np.angle(t1) * np.cos(j / 3) + np.angle(t2) * np.sin(j / 4)
            cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_610(t1, t2):
    try:
        n = 26
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n + 1):
            magnitude = np.log(np.abs(t1) + 1) * j**2 + np.log(np.abs(t2) +1) * (n - j +1)**1.5
            angle = np.angle(t1) * np.sin(j / n * np.pi) + np.angle(t2) * np.cos(j / n * np.pi) + np.sin(j) * 0.5
            cf[j - 1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j) + np.conj(t1) * t2**j
        for k in range(1, n + 1):
            cf[k - 1] = cf[k - 1] * np.exp(1j * np.sin(k * t1.real)) + np.exp(1j * np.cos(k * t2.imag))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_611(t1, t2):
    try:
        cf = np.zeros(26, dtype=np.complex128)
        for j in range(1, 27):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * t1.real) + np.cos(j * t2.imag)
            angle_part = np.angle(t1) * j**2 - np.angle(t2) * np.log(j + 1) + np.sin(j) * np.cos(j)
            intricate_sum = 0
            for k in range(1, j + 1):
                intricate_sum += (t1.real**k * np.cos(k)) / (k + 1)
            for r in range(1, int(np.floor(j / 2)) +1):
                intricate_sum += (t2.imag**r * np.sin(r)) / (r + 1)
            cf[j - 1] = (mag_part + intricate_sum * 1j) * np.exp(1j * angle_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_612(t1, t2):
    try:
        degree = 25
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            r_part = t1.real * np.sin(j * np.pi / 6) + t2.real * np.cos(j * np.pi / 7)
            i_part = t1.imag * np.cos(j * np.pi / 5) - t2.imag * np.sin(j * np.pi / 8)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * np.pi / 4)) * (1 + np.cos(j * np.pi / 9))
            angle = np.angle(t1) * np.sin(j * np.pi / 10) + np.angle(t2) * np.cos(j * np.pi / 11)
            cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        for j in range(1, degree + 2):
            cf[j - 1] += (t1.real - t2.imag) * np.sin(j * np.pi / 3) * np.cos(j * np.pi /5)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_613(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, num=25)
        imc = np.linspace(t1.imag, t2.imag, num=25)
        for j in range(1, 26):
            mag = np.log(np.abs(rec[j -1] + imc[j -1]) +1) * (j**2 + np.sin(j))
            ang = np.sin(rec[j -1] * j) + np.cos(imc[j -1] * j)
            cf[j -1] = mag * np.exp(1j * ang) + np.conj(t1) * t2**j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_614(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            mag_part1 = t1.real * j**2
            mag_part2 = np.log(np.abs(t1) + np.abs(t2) + 1) * (j + 1)
            mag = mag_part1 + mag_part2 + np.abs(t2.imag)**(j % 3 + 1)
            
            angle_part1 = np.angle(t1) * np.sin(j * np.pi / 4)
            angle_part2 = np.angle(t2) * np.cos(j * np.pi / 3)
            angle = angle_part1 + angle_part2 + np.sin(j)
            
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_615(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * t2.real) + np.log(np.abs(t2) + j) * np.cos(j * t1.imag)
            angle_part = np.angle(t1) * j + np.angle(t2) / (j + 1)
            cf[j -1] = mag_part * np.exp(1j * angle_part)
        for k in range(1, degree + 2):
            cf[k -1] += (t1.real - t2.imag) * np.sin(k * np.angle(t1)) + (t2.real + t1.imag) * np.cos(k * np.angle(t2))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_616(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for k in range(0, degree + 1):
            j = k + 1
            r_part = t1.real * np.sin(k * t2.real) + t2.real * np.cos(k * t1.real)
            im_part = t1.imag * np.cos(k * t2.imag) - t2.imag * np.sin(k * t1.imag)
            magnitude = np.log(np.abs(t1 + t2) + 1) * (k + 1) / (1 + k)
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k)
            cf[k] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_617(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            real_sum = 0
            imag_sum = 0
            for k in range(1, j +1):
                for r in range(1, k +1):
                    real_sum += (t1.real**k) * np.cos(r * np.angle(t2))
                    imag_sum += (t2.imag**r) * np.sin(k * np.angle(t1))
            cf[j -1] = complex(np.log(real_sum + 1), np.log(imag_sum + 1))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_618(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            r = j + 1
            mag = np.log(np.abs(t1) + r**2) * np.sin(r * t2.imag) + np.cos(r * t1.real)
            ang = np.angle(t1) * r + t2.real / (j + 1)
            cf[j] = mag * (np.cos(ang) + np.sin(ang) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_619(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, 10):
            mag = np.log(np.abs(t1 + j)**2 + np.abs(t2 - j)**2) * (1 + np.sin(j * np.angle(t1)) * np.cos(j * np.angle(t2)))
            angle = np.angle(t1) * j**0.5 - np.angle(t2) * (9 - j)**0.5 + np.sin(j * np.pi / 5)
            cf[j -1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_620(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, 10):
            mag_real = np.log(np.abs(t1) + 1) * np.sin(j * np.angle(t1))
            mag_imag = np.log(np.abs(t2) + 1) * np.cos(j * np.angle(t2))
            magnitude = mag_real + mag_imag
            angle = np.angle(t1)**j - np.angle(t2)**j
            cf[j -1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_621(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag = (t1.real**j + t2.imag**(j/2)) * np.log(np.abs(t1) + j) + np.sin(j * t2.real) * np.cos(j * t1.imag)
            angle = np.angle(t1) * np.sin(j) - np.angle(t2) * np.cos(j) + np.sin(j * t1.real) * np.cos(j * t2.imag)
            cf[j -1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_622(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag_part = np.log(np.abs(t1)**j + 1) * (t2.real + j)**2
            angle_part = np.angle(t1) * np.sin(j * np.angle(t2)) + np.cos(j * t2.real)
            cf[j -1] = mag_part * (np.cos(angle_part) + np.sin(angle_part) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_623(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for k in range(1, 10):
            j = k
            magnitude = np.log(np.abs(t1) + j) * (np.sin(j * t1.real) + np.cos(j * t1.imag))
            angle = np.angle(t1) * np.sin(j * t1.real) - np.angle(t2) * np.cos(j * t1.imag)
            cf[j -1] = magnitude * np.exp(1j * angle) + np.conj(t1) * (t2**j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_624(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t1)) + np.cos(j * t2.imag)
            angle_part = np.angle(t1)**j + np.angle(t2)**(j/2) + np.log(j + 1)
            mag = np.abs(mag_part) + np.prod([t1.real, t2.imag, j])
            angle = angle_part + np.sum([np.abs(t1), np.abs(t2), j])
            cf[j -1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_625(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag = np.log(np.abs(t1) + j) * (1 + np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2)))
            angle = np.angle(t1)**j - np.angle(t2)**(j/2) + np.sin(j * t1.real) * np.cos(j * t2.imag)
            cf[j -1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_626(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag_part1 = np.sin(j * np.pi * t1.real / (1 + j)) + np.cos(j * np.pi * t2.real / (1 + j))
            mag_part2 = np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1)
            magnitude = mag_part1 * mag_part2 + j**2
            angle_part1 = np.angle(t1) + np.angle(t2) * j
            angle_part2 = np.cos(j * t2.real) - np.sin(j * t1.imag)
            angle = angle_part1 + angle_part2
            cf[j -1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_627(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for k in range(1, 10):
            mag_part1 = np.log(np.abs(t1)**k + 1) * (t2.real + k)**2
            angle_part = np.angle(t1) * np.sin(k * np.angle(t2)) + np.cos(k * t2.real)
            mag_variation = mag_part1 * (np.abs(np.sin(k * t1.real)) + np.abs(np.cos(k * t2.real)))
            angle = angle_part
            cf[k -1] = mag_variation * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_628(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag = 0
            angle = 0
            for k in range(1, 5):
                mag += (t1.real**k * np.sin(k * j)) + (t2.imag**k * np.cos(k + j))
                angle += (np.angle(t1) + np.angle(t2)) / (k + j)
            cf[j -1] = mag * np.exp(1j * angle) + np.conj(t1) * np.log(np.abs(t2) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_629(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            r = t1.real * j
            im = t2.imag / j
            mag = np.log(np.abs(t1) + j) * (np.sin(r) + np.cos(im))
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j -1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_630(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            r_part = t1.real * np.log(np.abs(t2) + j) + np.cos(j * np.angle(t1 + t2))
            im_part = np.sin(j * np.angle(t1)) * np.abs(t2)**j + (t1.real + t2.real) / (j + 1)
            magnitude = np.sqrt(r_part**2 + im_part**2) * j**1.5
            angle = np.arctan2(im_part, r_part) + np.sin(j * np.pi / 3)
            cf[j -1] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_631(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            real_part = t1.real**j * np.log(np.abs(t2) + j) + np.cos(j * np.angle(t1 + t2))
            imag_part = np.sin(j * np.angle(t1)) * np.abs(t2)**j + (t1.real + t2.real) / (j + 1)
            cf[j] = real_part + 1j * imag_part
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_632(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            rec = t1.real + (t2.real - t1.real) * j / 8
            imc = t1.imag + (t2.imag - t1.imag) * j / 8
            mag = np.log(np.abs(rec * imc) + 1) * (j**2 + np.sin(j))
            angle = np.angle(t1) * j - np.angle(t2) * (9 - j) + np.cos(j * np.pi / 4)
            cf[j -1] = mag * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_633(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            r = j + 1
            mag = np.log(np.abs(t1) + r**2) * np.sin(r * t2.imag) + np.cos(r * t1.real)
            ang = np.angle(t1) * r + t2.real / (j + 1)
            cf[j] = mag * (np.cos(ang) + np.sin(ang) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_634(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        r1 = t1.real
        im1 = t1.imag
        r2 = t2.real
        im2 = t2.imag
        for j in range(1, 10):
            mag_part1 = np.sin(j * np.pi * r1 / (1 + j)) + np.cos(j * np.pi * r2 / (1 + j))
            mag_part2 = np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1)
            magnitude = mag_part1 * mag_part2 + j**2
            angle_part1 = np.angle(t1) + np.angle(t2) * j
            angle_part2 = np.cos(j * t2.real) - np.sin(j * im1)
            angle = angle_part1 + angle_part2
            cf[j -1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
def poly_635(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(0, 9):
            mag_part1 = np.log(np.abs(t1.real + j) + 1)
            mag_part2 = np.abs(t2)**(j / 3) + np.sin(j * np.pi / 5)
            angle_part1 = np.sin(j * t1.real) + np.cos(j * t2.imag)
            angle_part2 = np.angle(t1) * np.cos(j) - np.angle(t2) * np.sin(j)
            magnitude = mag_part1 * mag_part2 + np.prod([t1.real, t2.imag + j])
            angle = angle_part1 + angle_part2
            cf[j] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_636(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        r1 = t1.real
        i1 = t1.imag
        r2 = t2.real
        i2 = t2.imag
        for j in range(1, 10):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t2) / 2) + np.cos(j * np.angle(t1) / 3)
            angle_part = np.angle(t1) * np.cos(j) + np.sin(np.angle(t2))
            cf[j-1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_637(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag = np.log(np.abs(t1 * t2) + 1) * (t1.real**j + t2.imag**(j/2))
            angle = np.angle(t1) * np.sin(j) - np.angle(t2) * np.cos(j)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_638(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            rec_part = t1.real**j - t2.real**(9-j)
            im_part = t1.imag * t2.imag + np.cos(j * np.angle(t1) + np.sin(j * np.angle(t2)))
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * t1.real) * np.cos(j * t2.imag))
            angle = np.angle(t1) * j + np.angle(t2) / (j + 1)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_639(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            magnitude = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t1)) + np.log(np.abs(t2) + j**2) * np.cos(j * np.angle(t2))
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_640(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            k = j * 2
            r = 9 - j
            mag = np.log(np.abs(t1) + j) * (1 + np.sin(j)) + np.abs(t2)**(0.5 + np.cos(j))
            angle = np.angle(t1) * j + np.angle(t2) * k + np.sin(j) * np.cos(r)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_641(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            r_part = t1.real**j + t2.real**(9-j)
            im_part = t1.imag * np.sin(j) - t2.imag * np.cos(j)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j) * np.cos(j))
            angle = np.angle(t1) * j - np.angle(t2) / (j + 1) + np.sin(j * np.pi / 4)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_642(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            k = (j % 3) + 1
            r = (j // 2) + 1
            mag_part1 = np.log(np.abs(t1) + 1) * np.sin(j * np.pi / 4)
            mag_part2 = np.cos(k * np.pi / 3) * np.abs(t2)**r
            magnitude = mag_part1 + mag_part2
            angle_part1 = np.angle(t1) * j
            angle_part2 = np.sin(r * np.pi / 5) + np.angle(t2) * k
            angle = angle_part1 + angle_part2
            cf[j] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_643(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for k in range(1, 10):
            mag = t1.real**k + t2.imag**(9 - k) + np.log(np.abs(t1) + np.abs(t2) + 1) * np.sin(k * np.angle(t1) * np.angle(t2))
            angle = np.angle(t1) * np.cos(k * t2.real) - np.angle(t2) * np.sin(k * t1.imag)
            cf[k-1] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_644(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            part1 = t1.real**j + t2.imag**(j % 4 + 1)
            part2 = np.sin(j * t1.real + np.cos(j * t2.imag))
            part3 = np.log(np.abs(t1) + np.abs(t2) + j)
            magnitude = part1 * part2 + part3
            angle = np.angle(t1)**j + np.angle(t2) * np.sin(j) + np.angle(np.conj(t1)) - np.angle(np.conj(t2))
            cf[j-1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_645(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        real_seq = np.linspace(t1.real, t2.real, degree + 1)
        im_seq = np.linspace(t1.imag, t2.imag, degree + 1)
        for j in range(1, degree + 2):
            mag_component = np.log(np.abs(t1) + j) * np.sin(j * np.pi / 4) + np.cos(j * np.pi / 3) * np.abs(t2)
            angle_component = np.angle(t1) * j + np.angle(t2) * (degree + 1 - j)
            intricate_part = np.exp(1j * (np.sin(real_seq[j-1]) + np.cos(im_seq[j-1])))
            cf[j-1] = mag_component * intricate_part * np.conj(t2) + np.prod(np.arange(1, j+1)) * np.sin(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_646(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, 9)
        imc = np.linspace(t1.imag, t2.imag, 9)
        for j in range(1, 10):
            magnitude = np.log(np.abs(t1)**j + np.abs(t2)**(9-j) + 1) * (j**np.sin(j) + j**np.cos(j))
            angle = np.angle(t1) * np.sin(j * np.pi / 4) + np.angle(t2) * np.cos(j * np.pi / 3)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, 10):
            cf[k-1] = cf[k-1] * (1 + 0.1 * k) / (1 + 0.05 * k**2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_647(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for k in range(0, 9):
            j = k + 1
            r = t1.real + t2.real * k
            im = t1.imag - t2.imag * k
            angle = np.sin(r) * np.cos(im) + np.angle(t1 * t2) / (k + 1) - np.log(np.abs(r * im) + 1)
            mag = (np.abs(t1) * np.abs(t2))**k + (r + im + k) + (r * im * (k + 1))
            cf[k] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_648(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            sum_real = 0
            sum_imag = 0
            for k in range(1, j + 2):
                term_real = np.log(np.abs(t1) * k + 1) * np.sin(k * np.angle(t2))
                term_imag = np.log(np.abs(t2) * (degree - j + k) + 1) * np.cos(k * np.angle(t1))
                sum_real += term_real
                sum_imag += term_imag
            magnitude = sum_real**2 + sum_imag**2
            angle = sum_real / (sum_imag + 1e-8)
            cf[j] = np.sqrt(magnitude) * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_649(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            k = j + 1
            mag_part1 = np.log(np.abs(t1) + 1) * (j + 1)**1.5
            mag_part2 = np.log(np.abs(t2) + 1) * (degree - j + 1)**1.2
            magnitude = mag_part1 + mag_part2
            angle_part1 = np.sin(j * np.angle(t1)) 
            angle_part2 = np.cos(j * np.angle(t2))
            angle = angle_part1 + angle_part2
            cf[k-1] = magnitude * np.exp(1j * angle) + np.conj(t1) * np.conj(t2) * j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_650(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            mag_factor = np.log(np.abs(t1) + j) * np.sin(j * t2.real) + np.cos(j * t1.imag)
            angle_factor = np.angle(t1) * np.sqrt(j) - np.angle(t2) / (j + 1) + np.sin(j)
            cf[j-1] = mag_factor * np.exp(1j * angle_factor)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_651(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, 9)
        imc = np.linspace(t1.imag, t2.imag, 9)
        for j in range(1, 10):
            mag = np.log(np.abs(rec[j-1]) + 1) * np.cos(j) + np.abs(t2)**j
            ang = np.angle(t1) * np.sin(j * np.pi * imc[j-1]) + np.angle(t2) * np.cos(j * np.pi * rec[j-1])
            cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_652(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            angle_part = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude_part = np.abs(t1)**j + np.log(np.abs(t2) + 1) * j
            phase_variation = np.sin(j * t1.real) + np.cos(j * t2.imag)
            real_component = t1.real * magnitude_part * np.cos(angle_part) + phase_variation
            imag_component = t2.imag * magnitude_part * np.sin(angle_part) + phase_variation
            cf[j-1] = complex(real_component, imag_component)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_653(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, degree + 1)
        imc_seq = np.linspace(t1.imag, t2.imag, degree + 1)
        for k in range(1, degree + 2):
            mag = 0
            ang = 0
            for j in range(1, k + 1):
                mag += np.sin(rec_seq[j-1] * j) * np.cos(imc_seq[j-1] * j)
                ang += np.angle(rec_seq[j-1] + imc_seq[j-1] * 1j) * j
            mag = mag * np.log(np.abs(rec_seq[k-1] + imc_seq[k-1] * 1j) + 1)
            ang = ang / k
            cf[k-1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_654(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(0, 9):
            k = (j % 3) + 1
            r = np.log(np.abs(t1) + np.abs(t2)*j) * (j**1.5)
            angle = np.angle(t1)**k - np.angle(t2)**j + np.sin(j * np.pi / 5)
            cf[j] = r * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_655(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            if j % 2 == 0:
                mag = np.log(np.abs(t1) + j**2) * np.sqrt(j)
            else:
                mag = np.abs(t2)**j / (1 + j)
            if j % 3 == 0:
                angle = np.angle(t1) * np.sin(j) + np.cos(j * np.angle(t2))
            else:
                angle = np.sin(j * np.angle(t1)) - np.cos(j * np.angle(t2))
            cf[j-1] = mag * np.exp(1j * angle) * (t1.real + t2.imag / j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_656(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag_part1 = np.log(np.abs(t1) + 1) * np.sin(j * np.angle(t1))
            mag_part2 = np.log(np.abs(t2) + 1) * np.cos(j * np.angle(t2))
            magnitude = mag_part1 + mag_part2 + (j**2) / (np.abs(t1) + np.abs(t2) + 1)
            
            angle_part1 = np.angle(t1) * np.cos(j) 
            angle_part2 = np.angle(t2) * np.sin(j)
            angle_part3 = np.sin(j * t1.real) - np.cos(j * t2.imag)
            angle = angle_part1 + angle_part2 + angle_part3
            
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_657(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            r = j / degree
            mag = np.log(np.abs(t1) + np.abs(t2) + r) * (1 + np.sin(j * np.pi / 4))
            angle = np.angle(t1) * r**2 + np.angle(t2) * (1 - r)**2 + np.cos(j * np.pi / 3)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_658(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            mag = np.log(np.abs(t1) + np.abs(t2) + j + 1) * (j + 1)**1.5
            ang = np.sin(j * np.angle(t1)) - np.cos(np.angle(t2))
            cf[j] = mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_659(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for r in range(0, 9):
            mag = t1.real**2 * r + np.log(np.abs(t2) + 1) + np.sin(r * t1.real)
            angle = np.angle(t1) * r - np.cos(r * t2.imag)
            cf[r] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_660(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            mag_component = np.log(np.abs(t1) + np.abs(t2) + j + 1) * np.sin(j * np.pi / 5)
            angle_component = np.angle(t1) * np.cos(j) - np.angle(t2) * np.sin(j / 2)
            real_part = t1.real**(j % 3 + 1) + t2.real**(degree - j % 2)
            imag_part = t2.imag * np.cos(j * np.pi / 4)
            cf[j] = (mag_component + real_part) + 1j * (angle_component + imag_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_661(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            r_part = t1.real**j + t2.real**(9-j)
            i_part = t1.imag * np.sin(j) - t2.imag * np.cos(j)
            magnitude = np.log(r_part + 1) * np.abs(t1 + t2) + np.sin(r_part) * np.cos(i_part)
            angle = np.angle(t1)*j**2 - np.angle(t2)/j + np.sin(j * np.angle(t2))
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_662(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for k in range(0, degree + 1):
            j = np.sin(k * t1.real + np.cos(k * t2.imag)) + np.log(np.abs(t1) + np.abs(t2) + 1)
            r = np.cos(k * np.angle(t1)) * np.sin(k * np.angle(t2)) + t1.real * t2.imag
            magnitude = np.sqrt(j**2 + r**2) * (k + 1)
            angle = np.arctan2(r, j) + np.sin(k * t1.real * t2.imag)
            cf[k] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_663(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            r = j
            term1 = (t1.real**r + t2.imag**(r % 5 + 1)) * np.log(np.abs(t1) + 1)
            term2 = (np.abs(t2) * np.cos(r * np.angle(t1))) + (np.sin(r) * t2.real)
            angle = np.angle(t1) * np.sin(r) - np.angle(t2) * np.cos(r)
            cf[j-1] = (term1 + term2) * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_664(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(0,9):
            mag = np.log(np.abs(t1 + j * t2) + 1) * np.sin(j * np.pi / 4) + np.cos(j * np.angle(t1 * t2))
            angle = np.angle(t1)**j - np.angle(t2)**(8 - j) + np.sin(j * np.pi / 3)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_665(t1, t2):
    try:
        cf = np.zeros(8, dtype=np.complex128)
        for j in range(1, 9):
            mag = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t1))
            angle = np.cos(j * np.angle(t2)) + np.sin(j * t2.real)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_666(t1, t2):
    try:
        cf = np.zeros(8, dtype=np.complex128)
        rec1 = t1.real
        rec2 = t2.real
        imc1 = t1.imag
        imc2 = t2.imag
        for j in range(1, 9):
            r_part = np.log(np.abs(rec1 + j) + 1) * np.sin(j * np.pi / 4)
            i_part = np.log(np.abs(imc2 - j) + 1) * np.cos(j * np.pi / 3)
            magnitude = r_part + i_part
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_667(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 1):
            mag_sum = 0
            angle_sum = 0
            for k in range(1, j + 1):
                mag_sum += np.log(np.abs(t1) + k) * np.sin(k * np.angle(t2) + j)
                angle_sum += np.cos(k * np.pi / (j + 1))
            magnitude = mag_sum * (1 + j)
            angle = angle_sum + np.angle(np.conj(t1) * np.conj(t2)) * j**2
            cf[j] = magnitude * np.exp(1j * angle)
        cf[degree] = np.conj(t1) * np.conj(t2) + np.sum(np.abs(cf[0:degree]))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_668(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            k = j
            r_part = t1.real * np.log(k + 1) + t2.real * np.sin(k * np.pi / 7)
            i_part = t1.imag * np.cos(k * np.pi / 5) + t2.imag * np.log(k + 2)
            magnitude = np.sqrt(r_part**2 + i_part**2) * (1 + 0.1 * j)
            angle = np.arctan2(i_part, r_part) + np.cos(j * t2.real) * np.sin(j * t1.imag)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_669(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            term1 = t1.real**j * np.sin(j * np.angle(t2)) + t2.imag**(j % 5) * np.cos(j * t1.real - np.sin(np.abs(t2)))
            term2 = (t2.imag ** (j % 5)) * math.cos(j * t1.real - math.sin(j * np.abs(t2)))
            term3 = np.log(np.abs(t1) + np.abs(t2) + 1) * (t1.real * t2.imag)**(j % 3 + 1)
            magnitude = term1 + term2 + term3
            angle = np.angle(t1) * j - np.angle(t2) * (10 - j) + np.sin(j * t2.real)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_670(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (j**2 + np.sin(j))
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j**2)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1) * np.cos(j) - np.conj(t2) * np.sin(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_671(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            real_part = t1.real * j + t2.real * (degree - j)
            imag_part = t1.imag * np.sin(j) - t2.imag * np.cos(j)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j + 1) * (j + 1)
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        for k in range(1, degree + 2):
            cf[k-1] = cf[k-1] * np.exp(1j * np.sin(k * np.pi / 4)) + np.conj(cf[degree + 1 - k])
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_672(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            magnitude = np.log(np.abs(t1) + j) * (np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2)))
            angle = np.angle(t1) * np.sqrt(j) + np.angle(t2) / (j + 1)
            cf[j-1] = magnitude * np.exp(1j * angle) + np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_673(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            mag = np.log(np.abs(t1) + j * np.abs(t2) + 1) * (j + 1)**1.5
            angle = np.angle(t1)**j - np.sin(j * np.angle(t2)) + np.cos(j * t1.real * t2.imag)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_674(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        degree = 8
        for j in range(0, 9):
            mag_part1 = np.log(np.abs(t1) + 1) * np.sin(j * np.angle(t2))
            mag_part2 = np.cos(j * t1.real) * np.abs(t2)**0.5
            mag_part3 = t1.real * np.cos(t2.real) if j % 2 == 0 else t1.imag + t2.real
            magnitude = mag_part1 + mag_part2 + mag_part3
            angle_part1 = np.angle(t1)**j
            angle_part2 = t2.real * j
            angle_part3 = np.sin(j * np.angle(t1)) if j % 3 == 0 else np.cos(j * np.angle(t2))
            angle = angle_part1 + angle_part2 + angle_part3
            cf[j] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_675(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            mag_sum = 0
            angle_sum = 0
            for k in range(1, j + 1):
                mag_sum += np.log(np.abs(t1) + k) * np.sin(k * np.angle(t2) + j)
                angle_sum += np.cos(k * np.angle(t1 - t2))
            for r in range(1, (j % 3) + 2):
                mag_sum += np.prod([t1.real, t2.imag, r])
                angle_sum += np.angle(np.conj(t1) * np.conj(t2)) * r
            magnitude = mag_sum * (1 + j)
            angle = angle_sum + np.angle(t1) * j - np.angle(t2) * j**2
            cf[j-1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_676(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        r = t1.real
        m = t2.imag
        for j in range(1, 10):
            mag = (r**j + m**(j % 3 + 1)) * np.log(np.abs(t1) + np.abs(t2) + 1)
            angle = np.angle(t1)**j - np.angle(t2) + np.sin(j * r) * np.cos(j * m)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_677(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree, dtype=np.complex128)
        for j in range(1, degree + 1):
            r = t1.real * j / degree + t2.imag * (degree - j + 1) / degree
            theta = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            mag = np.log(np.abs(t1) + 1) * np.sin(j * np.pi / 4) + np.cos(j * np.pi / (degree + 1))
            cf[j-1] = mag * np.exp(1j * theta)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_678(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            k = j
            angle = np.angle(t1) * np.log(k + 1) + np.angle(t2) * np.sin(k)
            magnitude = np.abs(t1)**k + np.abs(t2)**(9 - k) + np.cos(k * np.pi / 4)
            cf[j-1] = magnitude * np.exp(1j * angle) + np.conj(t1) * np.sin(angle) * np.cos(k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_679(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree +1):
            mag_part = np.log(np.abs(t1) + j + 1) * np.abs(np.sin((j + 1) * np.angle(t1))) + \
                       np.log(np.abs(t2) + degree - j + 1) * np.abs(np.cos((j + 1) * np.angle(t2)))
            angle_part = np.angle(t1) * (j + 1) + np.angle(t2) * (degree - j) + np.sin(j)
            cf[j] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part)) + \
                    np.conj(t1 * t2) * (j + 1) / (degree + 1)
        for j in range(0, degree +1):
            cf[j] += np.conj(t1) * np.sin(j * t1.real) + np.cos(j * t2.imag)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_680(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(0, degree + 1):
            k = j + 1
            magnitude = np.log(np.abs(t1) + 1) * np.sin(j * np.angle(t2)) + np.cos(j * np.cos(j + 1))
            angle = np.angle(t1) * j - np.log(np.abs(t2) + 1) * np.cos(j * np.pi / 4)
            real_part = magnitude * np.cos(angle) + t1.real**(j % 3)
            imag_part = magnitude * np.sin(angle) + t2.imag**(j % 2 + 1)
            cf[j] = real_part + 1j * imag_part
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_681(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            k = (j * 3 + 2) % 8 +1
            r = np.log(np.abs(t1) + np.abs(t2)) * (j)
            mag = np.abs(t1)**j + np.abs(t2)**k + np.sin(j * np.angle(t1)) * np.cos(k * np.angle(t2))
            ang = np.angle(t1)*j - np.angle(t2)*k + np.sin(j * np.angle(t1)) + np.cos(k * np.angle(t2))
            cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_682(t1, t2):
    cf = np.zeros(8, dtype=np.complex128)
    for j in range(8):  # Python indexing starts at 0
        mag = 0
        ang = 0
        for k in range(1, j + 2):  # 1 to j inclusive in R translates to range(1, j+2) in Python
            mag += np.log(np.abs(t1 * k) + 1) * np.sin(k * np.pi / 4)
            ang += np.angle(t2) * np.cos(k * np.pi / 3)
        cf[j] = mag * np.exp(1j * ang) + np.conj(t1)**(j + 1) * np.imag(t2)
    return cf.astype(np.complex128)

def poly_683(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        rec1 = t1.real
        imc1 = t1.imag
        rec2 = t2.real
        imc2 = t2.imag
        for j in range(0, degree +1):
            mag = np.log(np.abs(t1) + j**1.5) * (1 + np.sin(j * np.pi / 3)) + np.cos(j * np.pi /4) * np.abs(t2)
            ang = np.angle(t1) * j + np.sin(j * np.pi /5) - np.cos(j * np.pi /6)
            cf[j] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_684(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1, 10):
            mag = np.log(np.abs(t1) + np.abs(t2) + j**2) * np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2))
            angle = np.angle(t1) * j - np.angle(t2) / (j +1)
            cf[j-1] = mag * np.cos(angle) + mag * np.sin(angle) * 1j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_685(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * np.pi / 3)) * (1 + np.cos(j * np.pi / 4))
            angle = np.angle(t1) * np.sqrt(j) + np.angle(t2) / (j +1)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_686(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree + 1, dtype=np.complex128)
        r1 = t1.real
        im1 = t1.imag
        r2 = t2.real
        im2 = t2.imag
        for j in range(1, degree +2):
            mag = np.log(np.abs(t1) + j) + np.sin(j * np.abs(t2)) * np.cos(j) + (t1.real**j) / (1 + j)
            angle = np.sin(j * r1) + np.cos(j * im2) + t2.imag / (j +1)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_687(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag = np.log(np.abs(t1)**j + np.abs(t2)**(9-j) + 1)
            ang = np.sin(j * t1.imag) + np.cos((9-j) * t2.real)
            cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_688(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            k = (j % 3) + 1
            r = (t1.real * j - t2.imag * k) / (j + k)
            mag = np.log(np.abs(t1) + 1) * np.sin(j * t2.real) + np.cos(k * t1.imag)
            ang = np.angle(t1)**j + np.angle(t2)**k + np.sin(j * k)
            cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_689(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for k in range(1,10):
            mag_part = np.log(np.abs(t1)**k + np.abs(t2)**(9 - k) + 1)
            angle_part = np.sin(k * np.angle(t1)) + np.cos((9 - k) * np.angle(t2))
            cf[k-1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_690(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(0, degree +1):
            mag_part = np.log(np.abs(t1) + np.abs(t2) + j) * np.sin(j * t2.real) + np.cos(j * t1.imag)
            ang_part = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            magnitude = mag_part + (t1.real + t2.imag) / (j + 1)
            angle = ang_part + np.sin(j * np.angle(t1)) * np.cos(j * np.angle(t2))
            cf[j] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_691(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            r_part = t1.real * j**np.sin(j) + t2.real / (j + 1)
            im_part = t1.imag * np.cos(j) + t2.imag * np.sin(j / 2)
            mag = np.log(np.abs(t1) + j) * np.abs(np.sin(j)) + np.log(np.abs(t2) + 1)
            angle = np.angle(t1) * j + np.angle(t2) * np.cos(j / 3)
            coeff = (r_part + 1j * im_part) * np.exp(1j * angle) * mag
            cf[j-1] = coeff + np.conj(t1) * np.sin(j) + np.cos(j) * np.sin(j / 2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_692(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            radius = np.log(np.abs(t1)**j + np.abs(t2)**(9-j) + 1)
            angle = np.sin(j * np.angle(t1)) + np.cos((9 - j) * np.angle(t2))
            cf[j-1] = radius * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_693(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,9):
            mag = np.log(np.abs(t1) + j**2) * np.sin(j * np.pi / 4) + np.cos(j * np.pi /6)
            ang = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j)
            cf[j-1] = mag * (np.cos(ang) +1j * np.sin(ang))
        cf[8] = t1.real**2 - t2.imag**2 + np.sin(t1.real)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_694(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag_part1 = np.log(np.abs(t1) + 1) * np.sin(j * np.pi / 5)
            mag_part2 = np.cos(j * t1.real) * np.abs(t2)**0.5
            mag_part3 = t1.real + t2.imag if j %2 ==0 else t1.real + t2.real
            magnitude = mag_part1 + mag_part2 + mag_part3
            
            angle_part1 = np.angle(t1)**j
            angle_part2 = t2.real * j
            angle_part3 = np.sin(j * np.angle(t1)) if j %3 ==0 else np.cos(j * np.angle(t2))
            angle = angle_part1 + angle_part2 + angle_part3
            
            base = magnitude * (np.cos(angle) + 1j * np.sin(angle))
            
            sum_part = np.sum(t1.real**np.arange(1,j+1)) + np.sum(t2.imag**np.arange(1,(j%2)+2))
            additional = sum_part * np.conj(t1 + t2)
            
            cf[j-1] = base + additional
        for k in range(1,10):
            cf[k-1] = cf[k-1] * (np.sin(k * t1.real) + np.cos(k * t2.imag)) * np.abs(t1 - t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_695(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,9):
            k = j +1
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (np.sin(j * t1.real) + np.cos(k * t2.imag))
            angle = np.angle(t1) * j - np.angle(t2) * k
            cf[j-1] = mag * np.exp(1j * angle)
        cf[8] = np.conj(cf[0]) + np.sum(cf[1:8]) * 0.5
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_696(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(0, degree +1):
            mag = np.log(np.abs(t1)**j + np.abs(t2)**(degree - j) + 1)
            ang = np.sin(j * np.angle(t1) + np.cos((degree - j) * np.angle(t2)))
            cf[j] = mag * np.exp(1j * ang) + np.conj(t1) * (j + 1)/(degree +1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_697(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(0, degree +1):
            mag_part = np.log(np.abs(t1) + 1) * (j +1)**np.sin(j) + np.sqrt(j +1) * np.cos(j * np.angle(t2))
            angle_part = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        for k in range(0, degree):
            cf[k+1] = cf[k+1] * np.exp(1j * (np.sin(k +1) + np.cos(k +1)))
        cf[0] = np.abs(t1) + np.abs(t2)
        cf[degree] = np.conj(t1) * np.conj(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_698(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag_part = np.log(np.abs(t1)**j + np.abs(t2)**(9-j) + 1)
            angle_part = np.sin(j * np.angle(t1)) + np.cos(angle_part)
            cf[j-1] = mag_part * (np.cos(angle_part) + 1j * np.sin(angle_part))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_699(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,9):
            r = t1.real * j + t2.real * (9 - j)
            im = t1.imag / (j +1) - t2.imag / (10 - j)
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (j**2)
            angle = np.sin(r) + np.cos(im) + np.angle(t1) * np.angle(t2)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle))
        cf[8] = np.conj(t1) + np.conj(t2) + np.sin(t1.real * t2.real)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_700(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (j**2 + np.sin(j))
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j**2)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_701(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(0, degree +1):
            r_part = t1.real * np.log(j + 2) + t2.real * np.sqrt(j +1)
            i_part = t1.imag * np.sin(j * np.pi /4) + t2.imag * np.cos(j * np.pi /3)
            magnitude = np.abs(t1)**(j %3 +1) + np.abs(t2)**(degree - j %2 +1)
            angle = np.angle(t1) * j + np.angle(t2) * (degree - j)
            cf[j] = (r_part +1j * i_part) * np.exp(1j * angle) + np.log(np.abs(magnitude) +1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_702(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, 9)
        imc = np.linspace(t1.imag, t2.imag, 9)
        for j in range(1,10):
            magnitude = np.log(np.abs(rec[j-1] + imc[j-1]) + 1) * np.sin(j * np.pi /4) + np.cos(j * np.pi /6)
            angle = np.angle(t1) * np.sin(2 * np.pi * j /9) + np.angle(t2) * np.cos(4 * np.pi * j /9)
            cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_703(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (j**2 + np.sin(j * t1.real) + np.cos(t2.imag))
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1) * t2.real * j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_704(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(0, degree +1):
            r = j +1
            mag = np.log(np.abs(t1) * j +1) * (np.abs(t2)**(degree - j +1)) + np.sin(j * t1.real)**2
            ang = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            real_part = mag * np.cos(ang) + t1.real**(j %3)
            imag_part = mag * np.sin(ang) + t2.imag**(j %2 +1)
            cf[j] = mag * np.exp( 1j * ang ) + ( np.conj(t1)**j ) * np.cos( j * np.real(t2) )
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_705(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag_variation = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(j * t1.real) * np.cos(j * t2.imag))
            angle_variation = np.angle(t1) * np.sqrt(j) - np.angle(t2) / (j +1)
            cf[j-1] = mag_variation * np.exp(1j * angle_variation) + np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_706(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag = 0
            angle = 0
            for k in range(1,j+1):
                mag += np.log(np.abs(t1.real + k)) * np.sin(k * t2.real)
                angle += np.cos(k * t1.imag) * np.angle(t2)**k
            cf[j-1] = mag * (np.cos(angle) + 1j * np.sin(angle)) + np.conj(t1)**j + np.conj(t2)**j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_707(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, 9)
        imc = np.linspace(t1.imag, t2.imag, 9)
        for j in range(1,10):
            mag = np.exp(np.sin(5 * np.pi * imc[j-1])) + np.log(np.abs(rec[j-1]) + 1) * np.cos(3 * np.pi * rec[j-1])
            ang = np.angle(t1) * np.sin(2 * np.pi * j /9) + np.angle(t2) * np.cos(4 * np.pi * j /9)
            cf[j-1] = mag * (np.cos(ang) +1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_708(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            k = j**2
            r = t1.real * np.cos(k * np.angle(t2)) + np.sin(k * t1.imag)
            im = t2.imag * np.log(k + np.abs(t1)) + np.cos(k * t1.real)
            mag = np.sqrt(r**2 + im**2) * (1 + j)
            angle = np.arctan2(im, r) + np.sin(j * np.angle(t1 + t2))
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_709(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(0, degree +1):
            mag_part = np.log(np.abs(t1) + np.abs(t2) +1) * (j +1)**np.sin(j * t1.real) + np.cos(j * t2.real)
            angle_part = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j] = mag_part * np.exp(1j * angle_part)
        for k in range(0, degree +1):
            cf[k] = cf[k] * (1 + 0.5 * np.sin(k * t1.imag) - 0.3 * np.cos(k * t2.imag))
        cf = cf * (1 + 0.1 * t1.real) / (1 + 0.1 * t2.imag)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_710(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(1, degree +2):
            sum_part = 0
            for k in range(1, j +1):
                sum_part += np.cos(k * t1.real) * np.sin(k * t2.imag)
            mag = np.log(np.abs(t1) + np.abs(t2) + sum_part +1)
            angle = (np.angle(t1)**0.5 * j) + (np.angle(t2)**0.3 * (degree - j +1))
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_711(t1, t2):
    try:
        cf = np.zeros(8, dtype=np.complex128)
        for k in range(1,9):
            r_part = t1.real**k + np.log(np.abs(t2))**2
            i_part = np.cos(k * t2.imag) + np.sin(k * np.angle(t1))
            angle = np.angle(t1) * k / 2 + np.sin(k) * np.pi /3
            cf[k-1] = (r_part + 1j * i_part) * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_712(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, degree +1)
        imc = np.linspace(t1.imag, t2.imag, degree +1)
        for j in range(1, degree +2):
            angle_component = np.sin(j * np.pi /3) * np.cos(j * np.pi /4)
            magnitude_component = np.log(np.abs(rec[j-1] * imc[j-1]) +1) + t1.real**j - t2.imag**(degree +1 -j)
            cf[j-1] = magnitude_component * np.exp(1j * (angle_component + np.angle(t1) * j))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_713(t1, t2):
    try:
        cf = np.zeros(8, dtype=np.complex128)
        for j in range(1,9):
            k = j * 2
            r = t1.real**j + np.log(np.abs(t2) +1) * np.sin(j * np.angle(t1))
            i_part = t2.imag**k - np.log(np.abs(t1) +1) * np.cos(j * np.angle(t2))
            mag = np.log(np.abs(t1)*np.abs(t2) +1) * j
            ang = np.angle(t1) + np.angle(t2) *j + np.sin(j) - np.cos(k)
            cf[j-1] = (r +1j * i_part) * mag * (np.cos(ang) +1j * np.sin(ang)) + np.conj(t1)**j * np.conj(t2)**k
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_714(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            r = t1.real * j**2 - t2.real / (j +1)
            im = t1.imag + t2.imag * np.sin(j)
            mag = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.abs(np.sin(j * np.pi /3)))
            ang = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j * np.pi /4)
            cf[j-1] = mag * np.exp(1j * ang) + np.conj(t1) * np.sin(j) - np.conj(t2) * np.cos(j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_715(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            rj = t1.real + j * t2.real
            ij = t1.imag - j * t2.imag
            mag = np.log(np.abs(rj + ij) +1) * (1 + np.sin(j * np.pi / 4 )) * (j**1.5)
            ang = np.angle(t1) * np.cos(j /3) + np.angle(t2) * np.sin(j /5)
            cf[j-1] = mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_716(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag = np.log(np.abs(t1) + j) * (np.sin(j * np.angle(t1)) + np.cos(j * np.angle(t2)))
            angle = np.angle(t1) * np.sqrt(j) + np.angle(t2) / (j +1)
            cf[j-1] = mag * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_717(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag = np.log(np.abs(t1)**j + np.abs(t2)**(j/2)) + np.sum(t1.real * t2.imag)
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(j) + np.sin(j * t1.real) * np.cos(j * t2.imag)
            cf[j-1] = mag * np.exp(1j * angle) + np.conj(t1 + t2) / (j +1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_718(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            k = j + 2
            r = t1.real * np.cos(j) - t2.imag * np.sin(j)
            im = t2.real * np.sin(j) + t1.imag * np.cos(j)
            magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (1 + np.sin(k))+np.cos(k)
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k)
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(9, dtype=np.complex128)

def poly_719(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(0, degree +1):
            k = j +1
            r = t1.real * np.sin(j * t1.real) + t2.imag * np.cos(k * t2.real)
            mag = np.log(np.abs(t1) + np.abs(t2) + j +1) * (j +1)**2
            angle = np.sin(r) + np.cos(k * np.pi /4)
            cf[j] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_720(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for k in range(0, degree +1):
            mag_part = np.abs(t1)**k * np.abs(t2)**(degree -k) + np.log(np.abs(t1) + np.abs(t2) +1)
            angle_part = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k)
            variation = np.sin(k * np.pi /3) + np.cos(k * np.pi /4)
            cf[k] = (mag_part * variation) * (np.cos(angle_part) +1j * np.sin(angle_part))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_721(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(1, degree +2):
            angle = np.sin(j * t1.real) + np.cos(j * t2.imag) + np.angle(t1) * j
            mag = np.log(j +1) * np.abs(t1)**j + np.abs(t2)**(degree +1 -j)
            cf[j-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        for k in range(1, degree +2):
            cf[k-1] = cf[k-1] * np.conj(t1) / (1 + k)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_722(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(1,10):
            mag_part = np.log(np.abs(t1)**j + np.abs(t2)**(9-j) + 1)
            angle_part = np.sin(j * np.angle(t1)) + np.cos((9-j) * np.angle(t2))
            real_part = mag_part * np.cos(angle_part) + t1.real * np.abs(t2)**j
            imag_part = mag_part * np.sin(angle_part) + t2.imag * np.abs(t2)**j
            cf[j-1] = complex(real_part, imag_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_723(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t2)) + np.cos(j * t1.imag)
            angle_part = np.angle(t1)**j - np.angle(t2)**(j %3) + np.sin(j * t1.real)
            cf[j-1] = mag_part * np.exp(1j * angle_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_724(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(1, degree +2):
            mag_part = np.abs(t1)**j * np.log(np.abs(t2) +1) + np.abs(t2)**(degree -j +1) * np.sin(j * t1.real)
            angle_part = np.angle(t1) * np.cos(j * t2.real) + np.angle(t2) * np.sin(j * t1.real)
            cf[j-1] = mag_part * np.exp(1j * angle_part) + np.conj(t1 * t2) / (j +1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_725(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            k = j %5 +1
            r = np.log(np.abs(t1) + j) * (1 + np.sin(j * t2.real) + np.cos(k * t1.imag))
            angle = np.angle(t1) * np.cos(j) + np.angle(t2) * np.sin(k)
            cf[j-1] = r * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_726(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for k in range(1,10):
            mag = np.log(np.abs(t1) + k**2) * np.sin(k * np.angle(t2)) + np.cos(k * t1.real)
            angle = np.angle(t1) * np.log(np.abs(t2) +1) + np.sin(k * t2.imag)
            cf[k-1] = mag * (np.cos(angle) +1j * np.sin(angle)) * (-1)**k
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
    
#function(t1, t2) {
#  n = 9
#  cf = complex(n)
#  rec = seq(Re(t1), Re(t2), length.out = n)
#  imc = seq(Im(t1), Im(t2), length.out = n)
#  for (k in 1:n) {
#    mag = log(Mod(t1) + Mod(t2) + k) * (k^2)
#    angle = Arg(t1) * sin(k) + Arg(t2) * cos(k)
#    cf[k] = mag * (cos(angle) + 1i * sin(angle))
#  }
#  cf

def bat(t1, t2):
    n = ps.poly.get('bat') or 11
    cf = np.arange(1, n + 1)
    mag = np.log(np.abs(t1) + np.abs(t2) + cf) * cf * cf
    angle = np.angle(t1) * np.sin(cf) + np.angle(t2) * np.cos(cf)
    return mag * (np.cos(angle) + 1j * np.sin(angle))

def batman(t1, t2, n=9):
    cf = np.arange(1, n + 1)
    mag = np.log(np.abs(t1) + np.abs(t2) + cf) * cf * cf
    angle = np.angle(t1) * np.sin(cf) + np.angle(t2) * np.cos(cf)
    return mag * (np.cos(angle) + 1j * np.sin(angle))

def batman10(t1, t2, n=10):
    cf = np.arange(1, n + 1)
    mag = np.log(np.abs(t1) + np.abs(t2) + cf) * cf * cf
    angle = np.angle(t1) * np.sin(cf) + np.angle(t2) * np.cos(cf)
    return mag * (np.cos(angle) + 1j * np.sin(angle))

def batman11(t1, t2, n=11):
    cf = np.arange(1, n + 1)
    mag = np.log(np.abs(t1) + np.abs(t2) + cf) * cf * cf
    angle = np.angle(t1) * np.sin(cf) + np.angle(t2) * np.cos(cf)
    return mag * (np.cos(angle) + 1j * np.sin(angle))

def batman12(t1, t2, n=12):
    cf = np.arange(1, n + 1)
    mag = np.log(np.abs(t1) + np.abs(t2) + cf) * cf * cf
    angle = np.angle(t1) * np.sin(cf) + np.angle(t2) * np.cos(cf)
    return mag * (np.cos(angle) + 1j * np.sin(angle))

def batman31(t1, t2, n=31):
    cf = np.arange(1, n + 1)
    mag = np.log(np.abs(t1) + np.abs(t2) + cf) * cf * cf
    angle = np.angle(t1) * np.sin(cf) + np.angle(t2) * np.cos(cf)
    return mag * (np.cos(angle) + 1j * np.sin(angle))

def batman51(t1, t2, n=51):
    cf = np.arange(1, n + 1)
    mag = np.log(np.abs(t1) + np.abs(t2) + cf) * cf * cf
    angle = np.angle(t1) * np.sin(cf) + np.angle(t2) * np.cos(cf)
    return mag * (np.cos(angle) + 1j * np.sin(angle))


def poly_727_old(t1, t2):
        try:
            n = 9
            cf = np.zeros(n, dtype=np.complex128)
            rec = np.linspace(t1.real, t2.real, n)
            imc = np.linspace(t1.imag, t2.imag, n)
            for k in range(1, n+1):
                mag = np.log( np.abs(t1) + np.abs(t2) + k ) * (k**2)
                angle = np.angle(t1) * np.sin(k) + np.angle(t1) * np.cos(k)
                cf[k-1] = mag * (np.cos(angle) +1j * np.sin(angle))
            return cf.astype(np.complex128)
        except:
            return np.zeros(0, dtype=np.complex128)

def poly_727(t1, t2):
    try:
        n = ps.poly.get('n') or 9
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, n)
        imc = np.linspace(t1.imag, t2.imag, n)
        for k in range(1, n+1):
            mag = np.log( np.abs(t1) + np.abs(t2) + k ) * (k**2)
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k)
            cf[k-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(9, dtype=np.complex128)

def poly_727_v1(t1, t2):
    try:
        n = 9
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, n)
        imc = np.linspace(t1.imag, t2.imag, n)
        for k in range(n):
            mag = np.log( np.abs(t1) + np.abs(t2) + k ) * (k**2)
            angle = np.angle(t1) * np.sin(k) + np.angle(t2) * np.cos(k)
            cf[k] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
        
def poly_727a(t1, t2):
    try:
        n = 9
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, n)
        imc = np.linspace(t1.imag, t2.imag, n)
        for k in range(1, n+1):
            mag = np.log( np.abs(t1) + np.abs(t2) + k ) * (k**3)
            angle = np.angle(t1) * np.sin(k) + np.angle(t1) * np.cos(k)
            cf[k-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)    
    
def poly_727b(t1, t2):
    try:
        n = 13
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, n)
        imc = np.linspace(t1.imag, t2.imag, n)
        for k in range(1, n+1):
            mag = np.log( np.abs(t1) + np.abs(t2) + k ) * (k**3)
            angle = np.angle(t1) * np.sin(k) + np.angle(t1) * np.cos(k)
            cf[k-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)  

def poly_727c(t1, t2):
    try:
        n = 29
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, n)
        imc = np.linspace(t1.imag, t2.imag, n)
        for k in range(1, n+1):
            mag = np.log( np.abs(t1) + np.abs(t2) + k ) * (k**3)
            angle = np.angle(t1) * np.sin(k) + np.angle(t1) * np.cos(k)
            cf[k-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_727d(t1, t2):
    try:
        n = 29
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, n)
        imc = np.linspace(t1.imag, t2.imag, n)
        for k in range(1, n+1):
            mag = np.log( np.abs(t1) + np.abs(t2) + k ) * (k**3)
            angle = np.angle(t1) * np.sin(k) + np.angle(t1) * np.cos(k)
            cf[k-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)  
            
def poly_727_alt(t1, t2):
    try:
        n = 9
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, n)
        imc = np.linspace(t1.imag, t2.imag, n)
        for k in range(1, n+1):
            mag = np.log( np.abs(t1) + np.abs(t2) + k ) * (k**2)
            angle = np.sin(k * np.angle(t1)) + np.cos(k)
            cf[k-1] = mag * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_728(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag = np.log(np.abs(t1) + j) * np.abs(np.sin(j * t1.real)) + np.sqrt(np.abs(t2.imag) + j)
            ang = np.angle(t1) * np.cos(j * np.angle(t2)) + np.sin(j * t2.real)
            cf[j-1] = mag * np.exp(1j * ang)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_729(t1, t2):
    try:
        degree = 8
        cf = np.zeros(degree +1, dtype=np.complex128)
        for j in range(0, degree +1):
            r_part = t1.real * j**2 - t2.real * np.sqrt(j +1)
            im_part = (t1.imag + t2.imag) * np.log(j +2)
            magnitude = np.abs(t1)**(j %3 +1) + np.abs(t2)**(degree -j)
            angle = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j] = (r_part +1j * im_part) * magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_730(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag_part = np.log(np.abs(t1) +1) * (j +1)**np.sin(j * t1.real) + np.cos(j * t2.real)
            angle_part = np.angle(t1) * np.sin(j) + np.angle(t2) * np.cos(j)
            cf[j-1] = mag_part * np.exp(1j * angle_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_731(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag_part = np.log(np.abs(t1) * j +1) * np.abs(np.sin(t1.real * j) + np.cos(t2.imag / (j +1)))
            angle_part = np.angle(t1) * np.sqrt(j) + np.angle(t2) / (j +2)
            cf[j-1] = mag_part * (np.cos(angle_part) +1j * np.sin(angle_part))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_732(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for k in range(0,9):
            r_part = t1.real * np.log(k +2) + t2.real * np.sqrt(k +1)
            i_part = t1.imag * np.sin(k) - t2.imag * np.cos(k)
            angle = np.sin(r_part) + np.cos(i_part)
            magnitude = np.log(np.abs(t1) + np.abs(t2) +k +1) * (k +1)
            cf[k] = magnitude * (np.cos(angle) +1j * np.sin(angle)) + np.conj(t1) * np.conj(t2)**k
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_733(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for k in range(1,10):
            tmp = 0 +0j
            for j in range(1,k+1):
                tmp += (t1.real**j / (j +1)) * np.exp(1j * np.sin(j * t2.real))
            for r in range(1, (k %3)+2):
                tmp += (t2.imag**r / (r +2)) * np.exp(1j * np.cos(r * t1.imag))
            cf[k-1] = tmp
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_734(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag = np.log(np.abs(t1) + j**2) * np.sin(j * np.angle(t2)) + np.cos(j * t1.real * t2.imag)
            angle = np.angle(t1) * np.cos(j * t2.real) + np.sin(j) * np.log(np.abs(t2) +1)
            cf[j-1] = mag * np.exp(1j * angle) + np.conj(t1) * t2**j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_735(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            real_part = t1.real**j + np.log(np.abs(t2) +1) *j
            imag_part = t2.imag * np.sin(j * np.angle(t1)) + np.cos(j * t2.real)
            magnitude = np.log(np.abs(t1 + t2) +j) * (1 +j**2)
            angle = np.angle(t1) * np.cos(j) + np.sin(j * np.angle(t2)) - np.cos(j * t1.imag)
            cf[j-1] = magnitude * (np.cos(angle) +1j * np.sin(angle))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_736(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        for j in range(1,10):
            mag_part = np.log(np.abs(t1) + j) * np.sin(j * np.angle(t2)) + np.cos(j * t1.real)
            ang_part = np.angle(t1) * np.cos(j) + np.sin(j * np.angle(t2))
            cf[j-1] = mag_part * np.exp(1j * ang_part)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_737(t1, t2):
    try:
        cf = np.zeros(9, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, 9)
        imc = np.linspace(t1.imag, t2.imag, 9)
        for k in range(1,10):
            mag = np.log(np.abs(t1) + np.abs(t2) +k) * (1 + np.sin(3 * np.pi * rec[k-1]) + np.cos(2 * np.pi * imc[k-1]))
            ang = np.angle(t1) * np.sin(5 * np.pi * imc[k-1]) - np.angle(t2) * np.cos(4 * np.pi * rec[k-1])
            cf[k-1] = mag * (np.cos(ang) +1j * np.sin(ang))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_738(t1, t2):
    try:
        r = t1.real * t2.imag
        i = t1.imag * t2.real
        
        cf = np.zeros(10, dtype=np.complex128)
        cf[0] = i * np.exp(1j * r) 
        cf[1] = 100j * np.sin(r) * np.cos(i)
        cf[2] = 1j * r + r * 1j
        cf[3] = np.abs(r * i) * np.exp(1j * np.arctan2(r, i))
        cf[4] = 100 / (1 + np.exp(-r))
        cf[5] = r * np.abs(i) + i * np.abs(r)
        cf[6] = np.sqrt(np.abs(r * i)**(1/3)) * np.exp(1j * (np.arctan2(r, i) + np.pi/4))
        cf[7] = r**2 + i**2
        cf[8] = np.abs(r +1j*i)**1.5 * np.exp(1j * (np.arctan2(r, i) + np.pi/4))
        cf[9] = r * i * np.abs(r - i)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_739(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = np.log(t1 + t2)
        cf[1] = np.exp(1j * t1)
        cf[2] = np.exp(-1j * t2)
        cf[3] = t1**2 - t2**2
        cf[4] = 1j * t1 * t2
        cf[5] = np.sin(t1) + np.cos(t2)
        cf[6] = np.cos(t1) - np.sin(t2)
        cf[7] = (t1 +1j*t2)**2
        cf[8] = (t2 -1j*t1)**3
        cf[9] = np.sqrt(t1**2 + t2**2)
        cf[10] = t1*t2*(t1 - t2)*(t1 +1j*t2)*(t2 -1j*t1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_740(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        r_1 = t1.real
        r_2 = t2.real
        i_1 = t1.imag
        i_2 = t2.imag
        
        cf[0] = i_1**3 - i_2**3 + r_1**2 - r_2**2
        cf[1] = r_1 * i_1 * r_2 * i_2 * (r_1 -1j*i_1) 
        cf[2] = i_1**2 + r_2**2 - i_2**2 + (r_2 - i_1)
        cf[3] = r_1 * (r_1**2 * i_1**2 - r_2**2 * i_2**2)
        cf[4] = r_1**3 + r_1**2 + i_2**3 + i_1**2 - 10
        cf[5] = i_1 * i_2 * (i_1**2 * r_2**2 - i_2**2 * r_1**2)
        cf[6] = r_1**0.5 - i_2**0.5 + r_2**0.5 - i_1**0.5
        cf[7] = r_1 * i_1**2 - r_2 * i_2 * (r_1 -1j*i_1)
        cf[8] = i_1**3 - i_2**3 + r_2**2 - r_1**2
        cf[9] = r_1 * i_1 * r_2 * i_2 * (i_2 - r_1) 
        cf[10] = i_1**4 + i_2**4 + r_1**4 + r_2**4
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_741(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = np.real((t1 +7j)**2) + np.imag((t2 +5j)**3)
        cf[1] = np.exp(1j*t1*t2)
        cf[2:6] = np.log(1j * np.array([2,3,4,5])) + np.exp(1j*t1) + np.exp(-1j*t2)
        cf[6:10] = cf[0:4][::-1]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_742(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(1, n+1):
            cf[k-1] = ((t1 +1j*t2)**k / np.math.factorial(k)) * np.exp(1j * np.sin(k*t2.real))
        cf[0] = t1**3 -1j*t1**2 + t2**2 -1j*t2
        cf[4] = t2.real * t1.imag -1j*t2**3
        cf[9] = t1.real**2 * t2.real**2 * np.exp(1j * (t1.real + t2.real))
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_743(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = np.sin(t1 * t2) * np.exp(1j * t1)
        cf[2] = t1**2 + t2**2
        cf[3] = np.exp(1j * t1**2) * np.cos(t2)
        cf[4] = (t1 +1j*t2) * (t1 -1j*t2)
        cf[5] = (t2 +1j*t1)**3 - (t1 -1j*t2)**3
        cf[6] = np.exp(-1j * t2**2) * np.sin(t1)
        cf[7] = np.real(t1**2 * t2**2 * np.exp(1j * (t1.real + t2.imag)))
        cf[8] = 100 * np.sin(t1) * np.cos(t2) - 100j * np.sin(t2) * np.cos(t1)
        cf[9] = 1j * (t1**3 - t2**3) + (100 -1j)
        cf[10] = (t1 + t2) * np.exp(2j * t1 * t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_744(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = t1**3 + t2**3
        cf[1] = 11*(t1 + t2)**9
        cf[2] = 0 +1j
        cf[3] = np.exp(1j*t1)
        cf[4] = 100 * np.sin(t2)
        cf[5] = t1.real -1j * t2.imag
        cf[6] = 11j * (t2.real / np.abs(t1.imag +0.1))
        cf[7] = t1.real / (np.abs(t1.real + t2.real) +0.125)
        cf[8] = np.exp(1j * t1.real * t2.real)
        cf[9] = np.abs(t1 * t2) * np.exp(1j*(np.angle(t1) - np.angle(t2)))
        cf[10] = t1.real * t2.imag +10j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_745(t1, t2):
    try:
        cf = np.zeros(10, dtype=np.complex128)
        cf[9] = t1.real * t2.imag
        cf[4] = t1.imag * t2.real
        cf[0] = np.exp(1j*(t1 + t2))
        m = np.abs(t1 + t2)
        cf[2] = 1 / (m +1)
        polar_coordinates = np.sqrt(t1.real**2 + t1.imag**2) * np.sqrt(t2.real**2 + t2.imag**2)
        cf[6] = np.exp(1j * polar_coordinates)
        cf[8] = np.sum(np.arange(1,10)**2) * t1.real
        cf[1] = t2**10 - cf[9]**10
        cf[3] = np.angle(t1) * cf[1]
        cf[5] = cf[2] + cf[6] * cf[8]
        cf[7] = np.conj(cf[3]) * cf[5]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_746(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        coeff_sequence = np.linspace(-1 + 2 * np.real(t1), 1 - 2 * np.imag(t2), num=11)
        for i in range(11):
            cf[i] = np.exp(1j * (i + 1) * t1) + np.sum(coeff_sequence[:i + 1] * np.log(1 + np.abs(t2))**2)
        cf[10] += np.sqrt(cf[0] * cf[1] * t1)
        cf[0] -= np.sqrt(cf[9] * cf[10] * t2)
        cf[5] = np.sum(cf) / 11
        cf[2] *= cf[7] / cf[5]
        cf[7] = cf[7]**2 - cf[4] + cf[8]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_747(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        for k in range(11):
            cf[k] = np.exp((t1 + t2 * 1j) ** (k + 1))
            if k % 2 == 1:
                cf[k] *= np.cos(k + np.imag(t2))
            else:
                cf[k] *= np.sin(k + np.real(t1))
            cf[k] += t1 * t2 * 1j * (k + 1) ** 2
            if cf[k] == 0:
                cf[k] = -1j
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_748(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = (t1 + t2) ** 2 - np.sqrt(np.abs(t1 * t2))
        cf[1] = np.log(1 + np.abs(t2)) * np.exp(1j * t1)
        cf[2] = np.real(t1) ** 2 - np.imag(t2) ** 2
        cf[3] = np.exp(1j * (np.real(t1) + np.real(t2)))
        cf[4] = 100 * np.exp(-np.abs(t1 - t2))
        cf[5] = (np.imag(t1) ** 3 - np.real(t2) ** 3) * np.exp(1j * (np.real(t1) + np.imag(t2)))
        cf[6] = np.sin(t1) - np.cos(t2)
        cf[7] = t2 ** 2 - t1 ** 2
        cf[8] = -10 * np.exp(1j * (t1 - t2) ** 2)
        cf[9] = np.exp(1j * t1 * t2) - np.sin(t1 * t2)
        cf[10] = (t1 + t2) ** 3 * np.exp(1j * (t1 - t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_749(t1, t2):
    try:
        cf = np.zeros(10, dtype=np.complex128)
        cf[0] = np.exp(1j * (t1 + t2))
        cf[1] = np.sin(t1) * np.cos(t2) - np.cos(t1) * np.sin(t2)
        for j in range(2, 9):
            cf[j] = np.exp((j + 1) / 3) * np.sin(t1 + t2) * np.exp(-1j * (t1 - t2) / (j + 1))
        cf[9] = np.sqrt(t1 ** 2 + t2 ** 2) - np.log10(np.abs(t1) + np.abs(t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_750(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = np.abs(t1 + t2) + 100j
        for k in range(1, 10):
            cf[k] = cf[k - 1] * (-1j * t1 + 1 * t2) ** (k + 1) / (k + 1)
        cf[10] = cf[0] * np.exp(1j * np.abs(np.sum(cf[:-1])))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_751(t1, t2):
    try:
        angle = np.linspace(0, 2 * np.pi, num=11)
        cf = 10 * np.exp(1j * angle)
        cf[0] = np.abs(t1 + t2)
        cf[5] = np.sin(t1) * (np.cos(t2) ** 2)
        cf[7] = -np.log(np.abs(t1) + 1) + 1j * np.log(np.abs(t2) + 1)
        cf[9] = (t1 ** 2) / (t2 + 1j)
        cf[10] = np.sqrt(t1 * t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_752(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = np.exp(1j * np.abs(t1)) + np.cos(np.real(t2) ** 2)
        cf[1] = 1j * np.sin(np.imag(t1 ** 2)) - t2
        cf[2] = (np.abs(t1) * np.abs(t2)) / 3
        cf[3] = (t1 + 1j * t2) / 2
        cf[4] = (t1 - 1j * t2) ** 2
        cf[5] = np.exp(1j * np.real(t1) * np.imag(t2)) - t1
        cf[6] = 1j * np.abs(t2 ** 3) + t1 ** 2
        cf[7] = np.exp(np.real(t1) ** 3 - 1j * np.imag(t2) ** 3)
        cf[8] = np.abs(t1 - t2) * (t1 + t2)
        cf[9] = t1 * np.abs(t2) - 1j * t1 * t2
        cf[10] = (t1 + t2) / (1 + t1 ** 2 + t2 ** 2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_753(t1, t2):
    try:
        cf = np.zeros(10, dtype=np.complex128)
        cf[0] = np.sin(t1 + t2) + np.cos(t1 - t2)
        cf[1] = np.exp(1j * (np.abs(t1 + t2) ** 2))
        prime_numbers = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29])
        for k in range(2, 8):
            cf[k] = prime_numbers[k] * ((t1 / (k + 1)) ** 2) * np.exp(1j * (t2 / (k + 1)))
        fibonacci_sequence = np.array([0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55])
        cf[8] = fibonacci_sequence[10] * t1 * t2 * np.exp(1j * (t1 - t2))
        cf[9] = ((t1 ** 3) + (t2 ** 3) - 1) * np.exp(1j * (t1 + t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_754(t1, t2):
    try:
        cf = np.zeros(10, dtype=np.complex128)
        cf[0] = 1j * t1 ** 2
        cf[1] = 1 / 2 * np.exp(-t2)
        cf[2] = t1 * t2 - t2 ** 3
        cf[3] = np.cos(t1) + np.cos(t2)
        cf[4] = np.sin(t1) * np.cos(t2)
        cf[5] = np.log(np.abs(t1 - t2) + 1)
        cf[6] = (t1 + t2) ** 2
        cf[7] = np.real(t1) ** 3 + np.imag(t2) ** 3
        cf[8] = np.abs(t1 * t2)
        cf[9] = np.exp(1j * (t1 * t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_755(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = (t1 * np.conj(t2)) ** 3 + t1 - t2
        cf[10] = t2 + 1j * t1
        snd = np.linspace(np.sin(2 * np.pi * np.real(t1)), np.sin(2 * np.pi * np.imag(t2)), num=9)
        csi = np.linspace(np.cos(2 * np.pi * np.imag(t1)), np.cos(2 * np.pi * np.real(t2)), num=9)
        stat = t1 * csi ** 2 + t2 * snd ** 2 + 1j * (t1 * snd ** 2 + t2 * csi ** 2)
        cf[1:10] = stat
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_756(t1, t2):
    try:
        cf = np.zeros(10, dtype=np.complex128)
        cf[0] = np.exp(1j * t1 * t2)
        cf[1] = 100 * np.sin(t1 * t2)
        cf[2] = 100j * np.cos(t1 * t2)
        cf[3] = 1j * t1 ** 3 - 2j * t1 * t2 ** 2
        cf[4] = t1 ** 5 + t2 ** 5
        cf[5] = 10j * t1 ** 4 - 10j * t2 ** 4
        for k in range(6, 10):
            cf[k] = cf[k - 1] * 1j * 0.8 ** (k + 1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_757(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = 0
        cf[1] = np.sqrt(np.abs(t1))
        cf[2] = np.cos(t1) + np.sin(t2)
        cf[3] = t2 / (np.abs(t1) + 1)
        cf[4] = np.exp(1j * np.angle(t1)) - np.exp(-1j * np.angle(t2))
        cf[5] = cf[2] ** 2 - cf[3] ** 2
        cf[6] = (t1 * t2 * cf[1]) / (1 + cf[1] ** 2)
        cf[7] = np.real(t1) + np.imag(t2) - (np.real(t2) + np.imag(t1))
        cf[8] = cf[7] / (1 + np.abs(cf[5]))
        cf[9] = cf[4] * cf[5] * cf[6]
        cf[10] = cf[8] ** 3 + cf[9] ** 2
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_758(t1, t2):
    try:
        cf = np.zeros(10, dtype=np.complex128)
        cf[0] = (t1 ** 2 + t2 ** 2) * np.exp(1j * np.angle(t1 + t2))
        cf[1] = 100j * (t1 ** 3 - t2 ** 2) * np.cos(np.angle(t1 - t2))
        cf[2] = np.real(t1 - t2) ** 2 + np.imag(t1 + t2) ** 2 - 100
        cf[3] = 42 * (np.log(1 + np.abs(t1)) + np.log(1 + np.abs(t2))) * np.exp(1j * np.pi / 4)
        cf[4] = np.sqrt(np.abs(t1) + np.abs(t2)) * np.exp(1j * (np.angle(t1 * t2) - np.pi / 3))
        cf[5] = np.sinh(0.1 * np.real(t1 + t2)) + np.cosh(0.1 * np.imag(t1 - t2))
        cf[6] = 1 / (1 + np.exp(-np.abs(t1))) + 1j / (1 + np.exp(-np.abs(t2)))
        cf[7] = np.arctan(1 / np.abs(t1 + t2)) * 1j
        cf[8] = 1j * t1 ** 3 - t2 * 2 - 200
        cf[9] = np.exp(1j * t1 * t2) / (1 + t1 + t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_759(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = np.exp(t1) * np.cos(t2)
        cf[1] = -np.exp(1j * t1)
        cf[2] = np.log(np.abs(t2))
        cf[3] = (t1 + t2) / (1j)
        cf[4] = t1 ** 3 - t2 ** 3
        cf[5] = -2 * np.exp(t2) * np.sin(t1)
        cf[6] = np.sqrt(np.abs(t1 - t2))
        cf[7] = np.exp(1j * (t1 + t2) ** 2)
        cf[8] = np.log1p(np.abs(t1 * t2))
        cf[9] = (t1 - t2) / (1j)
        cf[10] = np.exp(t1 * t2) / t1
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_760(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = np.sin((np.real(t1) + np.imag(t2) * 2) / (1 + np.abs(t1 + t2) ** 2))
        cf[1] = 100 * np.real(t1) * np.imag(t1) * (np.abs(np.cos(np.real(t2))) ** 2.1 - np.abs(np.sin(np.imag(t2))) ** 2.1)
        mm = np.array([[np.real(t2), np.imag(t2)], [-np.imag(t1), np.real(t1)]])
        if np.abs(np.linalg.det(mm)) < 1e-10:
            vv = 0
        else:
            vv = np.sum(np.linalg.inv(mm))
        cf[2] = 1000 * vv
        cf[3] = np.sum(np.fft.fft(np.array([t1, t2]), inverse=True))
        cf[4] = 10 * np.exp(1j * np.arctan2(np.imag(t1), np.real(t1))) * np.sqrt(np.real(t1) ** 2 + np.imag(t1) ** 2)
        cf[5] = 1000 * np.median([np.real(t1), np.imag(t1), np.real(t2), np.imag(t2)]) ** 2 + 500 * np.median([np.real(t1), np.imag(t1), np.real(t2), np.imag(t2)]) ** 3
        cf[6] = np.real(t2) + 1j * 2 * np.sqrt(np.abs(np.imag(t2)))
        cf[7] = 100 * np.log10(np.abs(np.real(t2)) + np.abs(np.imag(t2)))
        cf[8] = np.sqrt(np.abs(t1 * t2))
        cf[9] = 1000 * (np.imag(t1) ** 3 - 3 * np.imag(t1) * np.real(t1) ** 2)
        cf[10] = 2j * (np.real(t1) - np.imag(t1)) + 2 * (np.real(t1) + np.imag(t1))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_761(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(n):
            cf[k] = (100 * np.sin(np.real(t1) * (k + 1)) * np.cos(np.imag(t2) * (k + 1))) * np.exp(1j * (np.real(t2) * (k + 1) / 100))
        cf[0] *= 10
        cf[4] *= np.sin(t1 * t2)
        cf[9] += np.cos(t1 * t2) * np.exp(1j * (np.real(t1) + np.real(t2)))
        return cf.astype(np.complex128)
    except Exception:
        return np.full(n,0, dtype=np.complex128)

def poly_762(t1, t2):
    try:
        cf = np.full(10, 0,dtype=np.complex128)
        primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29],dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1:9] = primes[1:9] * (t1 ** 2 + t2 ** 2 + 1j)
        cf[9] = np.sum(primes[:9]) + np.abs(t1 ** 2 + t2 ** 2)
        return np.array(cf,dtype=np.complex128)
    except Exception:
        return np.full(10,0, dtype=np.complex128)

def poly_763(t1, t2):
    try:
        cf = np.zeros(10, dtype=np.complex128)
        cf[0] = (t1 ** 2 + t2 ** 2) * 1j
        cf[1] = 10
        cf[4] = np.abs(t1 * 100) - 0.5
        cf[5] = np.abs(t2 * 100) - 0.5
        cf[7] = -10
        cf[9] = (t1 ** 2 + t2 ** 2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_764(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = np.exp(1j * t1)
        cf[1] = (t1 + t2) * np.cos(t1) + 1j * np.sin(t2)
        cf[2] = t1 ** 3 * t2 ** 2 - 1j * t1 ** 2 * t2 ** 3
        cf[3] = np.log(t1 + 1j * t2)
        cf[4] = t1 * np.cos(t1) + t2 * np.sin(t2)
        cf[5] = t1 ** 2 * t2 - t1 * t2 ** 2
        cf[6] = 1j * t1 ** 3 + t2 ** 3
        cf[7] = (t1 + 1j * t2) ** 3 - t1 * t2
        cf[8] = t1 * t2 * (t1 - t2) * (t1 + t2)
        cf[9] = t1 ** 3 * t2 ** 2 * np.exp(1j * (t1 - t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_765(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = 100 * np.real(t1 ** t2)
        cf[2] = 10 * np.real(t1 * t2 ** 3)
        cf[3] = 200 * np.imag(t1 ** t2)
        cf[4] = np.exp(1j * t1)
        cf[5] = np.sign(np.real(t1)) * np.abs(t2)
        cf[6] = (np.real(t1) ** 2 + np.imag(t2) ** 2) * np.exp(1j * np.angle(t1))
        cf[7] = np.where(np.real(t1) > np.imag(t2), 1 + 1j * t2, 100j + t1)
        cf[8] = t1 + 1j
        cf[9] = 10 * (0.1 + np.exp(1j * t2))
        cf[10] = 0.001 + 1j * t2 ** 3
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_766(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = 100 * np.exp(1j * np.abs(t1))
        cf[1] = 10 * (np.real(t1) ** 2 - np.imag(t2) ** 2)
        cf[2] = 1j * (np.real(t2) + np.imag(t1)) * (np.real(t1) - np.imag(t2))
        cf[3] = (t1 + t2) ** 2 - (t1 - t2) ** 2
        cf[4] = 100 * (t1 + t2) / (1 + np.abs(t1 * t2))
        cf[5] = sum((t1**k).real * (t2**k).imag for k in range(1, 6))
        cf[6] = np.sqrt(np.abs(t1 ** 2 - t2 ** 2))
        cf[7] = t1 * t2 / (1 + np.abs(t1 - t2))
        cf[8] = (math.prod(range(math.floor(t1.real), math.floor(t2.imag)+1)) if t1.real <= t2.imag else math.prod(range(math.floor(t1.real), math.floor(t2.imag)-1, -1))) + 1j * (math.prod(range(math.floor(t2.real), math.floor(t1.imag)+1)) if t2.real <= t1.imag else math.prod(range(math.floor(t2.real), math.floor(t1.imag)-1, -1)))
        cf[9] = np.exp(1j * (np.real(t1) - np.imag(t2)))
        cf[10] = np.exp(-1j * (np.real(t2) - np.imag(t1)))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_767(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = np.real(t1) * np.imag(t2) + t1 * t2 + 1j
        cf[1] = 3j * t1 ** 2 + 2j * t2 ** 2 - 2 * t2 + 1j
        cf[2] = np.sin(t1 ** 3 + t2 ** 3) + np.cos(t1 * t2) + 2
        cf[3] = np.exp(1j * t1 * t2) + 2j * t1 - t2 + 2
        cf[4] = t1 ** 3 - 3 * t2 + np.imag(t1 + t2) ** 2 + 1j
        cf[5] = np.real(t1) * np.sin(t2) + t1 / (t2 + 1) + 2j
        cf[6] = np.cos(t1) / (1 + np.abs(t2)) + np.sin(t1 + t2) + 2
        cf[7] = np.exp(1j * t1) - np.exp(1j * t2) + np.sqrt(np.abs(t1 + t2)) + 1j
        cf[8] = np.abs(t1) * np.abs(t2) - np.sin(np.real(t1)) * np.cos(np.imag(t2)) + 1j
        cf[9] = t1 ** 2 * (t2 + 1) - (t1 + 1) * t2 ** 2 + 1j
        cf[10] = 1j * t2 + np.real(t1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_768(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 100 * (t1 ** 4 - t2 ** 4) * np.exp(1j * (t1 - t2) ** 4)
        cf[1] = 50j * (t1 + t2 ** 3)
        cf[2] = 100 * (t1 - t2) * np.exp(-1j * t1 ** 2) + 50 * (t1 + t2) ** 2
        cf[3] = -25j * (t1 - 1j * t2) ** 3
        cf[4] = (t1 + 3j * t2) ** 4 + 50j * (t1 + 2j * t2) ** 2
        cf[5] = 75 * (t1 - 1j) ** 4 + 100 * (2j * t2 + t1) ** 2
        cf[6] = -100j * np.exp(1j * np.abs(t1 * t2))
        cf[7] = 50j * (t1 ** 2 + t2 ** 2) ** 2 + 25 * (t1 - 1j) ** 4
        cf[8] = 75 * np.exp(-1j * t1 ** 2) + 100j * t2 ** 4
        cf[9] = 100 * (t1 * t2) ** 4 - 25j * ((t1 + t2) ** 3 + t1 ** 2 * t2 ** 2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_769(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 100 * np.sin(t1) ** 3 * np.cos(t2) ** 2
        cf[1] = 100 * np.exp(1j * (t1 + t2)) - 10 * (t1 - t2) ** 2
        cf[2] = t1 * t2 * (t1 - t2) / (np.abs(t1) + np.abs(t2) + 1)
        cf[4] = (t1 * t2 * np.exp(1j * (t1 ** 2 - t2 ** 2))) ** 3
        cf[6] = np.sqrt(np.abs(t1)) - np.sqrt(np.abs(t2)) + 1j * np.sin(t1 * t2)
        cf[7] = 50 * np.abs(t1 - t2) * np.exp(1j * np.abs(t1 + t2))
        cf[8] = np.where(np.imag(t1) > 0, t1 - np.abs(t2), t2 - np.abs(t1))
        cf[9] = (1j * t1 * t2) ** (0.1 * t1 * t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_770(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = t1 ** 2 - t2 ** 2
        cf[2] = t1 ** 3 + t2 ** 3
        cf[3] = np.sin(t1) * np.cos(t2)
        cf[4] = np.exp(1j * (t1 - t2))
        cf[5] = np.log(np.abs(t1 + t2))
        cf[6] = t1 ** 4 + 1j * t2 ** 4
        cf[7] = (t1 * t2) ** 2
        cf[8] = (t1 + t2) / 2
        cf[9] = t1 ** 5 - t2 ** 5
        cf[10] = np.exp(1j * (t1 * t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_771(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = 100 * t1 ** 3 + 200j * t2 ** 2 - 100 * t1 - 55j
        cf[1] = 1j * (t1 ** 2 + t2 ** 2) ** 3 + 2j * t2 ** 5
        cf[2] = (t1 + t2) ** 3 - (t1 - t2) ** 2 + np.sin(t1 + t2)
        cf[3] = np.exp(1j * t1 ** 2 * t2 ** 3)
        cf[4] = np.log(1 + np.abs(t1 * t2)) - 50 * t1 * t2
        cf[5] = np.abs(t1 - t2) * (t1 + t2) * np.log(1 + np.abs(t1 + t2))
        cf[6] = np.exp(-1j * t1 ** 2) * (t1 + t2) ** 3 - (t1 - t2) ** 3
        cf[7] = 100 * t2 ** 3 + 200j * t1 ** 2 - 100 * t2 - 55j
        cf[8] = (t1 ** 2 + t2 ** 2) * np.sin(t1 * t2)
        cf[9] = 2 * (np.abs(t1) + np.abs(t2)) ** 4 + 1j * (t1 ** 2 - t2 ** 2)
        cf[10] = 50 * (t1 + t2) ** 2 + 50 * t1 * t2 - 50j * (t1 - t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_772(t1, t2):
    try:
        m = 10
        cf = np.zeros(m + 1, dtype=np.complex128)
        cf[[0, m]] = [t1, t2]
        cf[[1, m - 1]] = [1j * t1, -1j * t2]
        for x in range(1, m - 1):
            cf[x + 2] = np.abs(cf[x + 1] * cf[m - x + 1]) * np.cos(cf[0] * cf[m]) ** (x + 1)
        mult = np.exp(1j * np.linspace(0, 2 * np.pi, num=m + 1))
        cf *= mult
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_773(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = np.sin(t1) + 1j * np.cos(t2)
        cf[1] = -1j * t1 + t2
        cf[2] = np.exp(1j * (t1 - t2)) + np.log(1 + np.abs(t1) + np.abs(t2))
        cf[3] = (t1 * t2) ** 2 - 1j * (t1 - t2) ** 2
        cf[4] = 100j * np.sin(t2) + 100 * np.cos(t1)
        cf[5] = (t1 + t2) ** 3 - 1j * (t1 - t2) ** 3
        cf[6] = np.exp(1j * (t1 * t2))
        cf[7] = np.log(1 + np.abs(t1 * t2)) + 1j * (t1 - t2)
        cf[8] = (t1 ** 2 + t2 ** 2)
        cf[9] = (t1 + 1j * t2) ** 3
        cf[10] = np.prod(np.cos(np.linspace(1, 10, num=100)) + 1j * np.sin(np.linspace(1, 10, num=100))) * t1 * t2
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_774(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        complex_sum = t1 + t2
        complex_product = t1 * t2
        complex_diff = t1 - t2
        cf[0] = np.exp(1j * np.angle(complex_sum)) * np.abs(complex_sum)
        cf[1] = np.real(complex_product) + 1j * np.imag(complex_diff)
        cf[2] = np.abs(complex_product)
        cf[3] = cf[0] + cf[1] + cf[2]
        cf[4] = cf[0] * cf[2] - cf[1] * cf[3]
        cf[5] = 10 + np.sin(cf[4]) + np.sin(cf[3]) + np.cos(cf[2]) + np.cos(cf[1]) + np.sin(cf[0])
        cf[6] = 2j * cf[0] + cf[1] / cf[5]
        cf[7] = cf[1] * cf[2] * cf[3] + cf[0]
        cf[8] = 2 * cf[7] - cf[4] * cf[6]
        cf[9] = cf[0] * cf[1] * cf[2] * cf[3] * cf[4] * cf[5] * cf[6] * cf[7] * cf[8]
        cf[10] = (cf[0] + cf[2] - cf[4] + cf[6] - cf[8]) / (cf[1] - cf[3] + cf[5] - cf[7] + cf[9])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_775(t1, t2):
    try:
        cf = np.zeros(11, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = (t1 ** 3 + t2 ** 2) * 1j
        cf[2] = t1 ** 2 * np.cos(np.abs(t2))
        cf[3] = t2 ** 3 * np.sin(np.angle(t1))
        cf[4] = np.real(np.exp(1j * t1) * np.exp(-1j * t2))
        cf[5] = np.imag((t1 + t2) ** 2 * 1j)
        cf[6] = np.abs(t1 - t2) * np.cos(t1 + t2)
        cf[7] = np.angle(t1 * t2) * np.sin(t1 * t2) * 1j
        cf[8] = (t1 + t2) ** 3 - np.real(t1 ** 2 * t2 ** 3 * 1j)
        cf[9] = np.abs(t1 ** 2 - t2 ** 2) * np.exp(1j * np.angle(t1 + t2))
        cf[10] = (np.real(t1) + np.imag(t2)) ** 2 * np.sin(np.abs(np.real(t1) - np.imag(t2)))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_776(t1, t2):
    try:
        n = 10
        cf = np.zeros(n, dtype=np.complex128)
        cf[0] = 1 + np.sin(t1 + t2)
        cf[1] = t1 * t2 * np.exp(-1j * t2)
        cf[2] = 100 * np.where(np.abs(t1 - t2) < 1, t1, t2)
        cf[3] = 200 * (np.real(t1) - np.imag(t1))
        cf[4] = -100j * t2 + 100 * t1 ** 2
        cf[5] = np.tan(t1 / (1 + np.abs(t2)))
        cf[6] = 100j * np.real(t1 * t2 * np.exp(1j * (t1 - t2)))
        cf[7] = -100 * np.sin(t2) ** 3 + t1 ** 2 * np.cos(t2) * np.sin(t1)
        cf[8] = (t1 + t2) ** 4 - (t1 - t2) ** 4
        cf[9] = cf[1] * cf[7] - cf[0] * cf[8]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_777(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[1] = t1 + t2
        for k in range(2, 26):
            v = np.exp(1j * np.angle(cf[k - 1] + t2)) * np.abs(k * cf[k - 1] + t1)
            cf[k] = v
        cf[9] = np.real(t1) + np.imag(t2)
        cf[14] = 1j * (np.real(t1) + np.imag(t2))
        cf[19] = np.real(t1 * t2) * (1 + 1j)
        cf[24] = np.abs(t1) ** 2 + np.abs(t2) ** 2
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_778(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.abs(t1 + t2)
        cf[1] = 2 * np.real(t1) * np.imag(t2)
        cf[2] = np.angle(t1 + t2)
        cf[3] = np.conj(t1) * t2
        cf[4] = np.angle(t1) * np.angle(t2)
        for k in range(5, 22):
            cf[k] = np.abs(t1 + (-1) ** k * t1 ** 2 / (k + 1) + (-1) ** k * t2 ** 2 / (k + 1))
        cf[21] = cf[1] + cf[2] - cf[3] + cf[4]
        cf[22] = np.abs(cf[1] * cf[2] * cf[3] * cf[4])
        cf[23] = 1 + np.real(np.conj(t1) * t2)
        cf[24] = 1j + np.imag(np.conj(t1) * t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_779(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            cf[k] = np.abs(t1) ** (k + 1) + np.angle((t2 + 1j) ** (k + 1)) + np.log(np.abs(np.sin(k * t1) * np.cos(k * t2)))
        cf[4] = t1 ** t2 + np.conj(t2) ** 3
        cf[14] = np.cos(np.real(t1)) * np.sin(np.imag(t2)) + np.log(np.abs(1j * t2))
        cf[24] = np.real(np.conj(t2) ** t1) - np.imag(1j * t1 ** 3)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_780(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 ** 2 + t2 ** 2 - t1 * t2
        for k in range(1, 25):
            cf[k] = cf[k - 1] * (t1 + t2) / (1 + np.abs(cf[k - 1]))
        cf[[2, 5, 8, 11, 14, 17, 20, 23]] += (t1 + 1j * t2)
        cf[[1, 4, 7, 10, 13, 16, 19, 22, 24]] -= (t2 + 1j * t1)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_781(t1, t2, err = False):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1 ** 2 + t2 ** 2
        for k in range(2, 13):
            cf[k] = (cf[k - 1] * cf[k - 2]) / (1 + np.abs(t1) + np.abs(t2))
        for l in range(13, 25):
            cf[l] = (cf[l - 1] + cf[l - 13]) / (1 + np.real(t1) ** 2 + np.imag(t2) ** 2)
        return cf.astype(np.complex128)
    except Exception as e:
        if err:
            print(f"Error occurred: {e}")
            return np.zeros(25, dtype=np.complex128)
        else:
            return np.zeros(25, dtype=np.complex128)
        
def poly_781_v1(t1, t2, err = False):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = t1 ** 2 + t2 ** 2
        for k in range(3, 14):
            cf[k] = (cf[k - 1] * cf[k - 2]) / (1 + np.abs(t1) + np.abs(t2))
        for l in range(14, 25):
            cf[l] = (cf[l - 1] + cf[l - 13]) / (1 + np.real(t1) ** 2 + np.imag(t2) ** 2)
        return cf.astype(np.complex128)
    except Exception as e:
        if err:
            print(f"Error occurred: {e}")
            return np.zeros(25, dtype=np.complex128)
        else:
            return np.zeros(25, dtype=np.complex128)
        
def poly_782(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = np.exp(1j * np.angle(t1 * np.conj(t2)))
        cf[2] = np.abs(t1) * np.abs(t2)
        for k in range(3, 26):
            cf[k] = (np.real(cf[k - 1]) + 1j * np.imag(cf[k - 1])) * np.exp(1j * np.angle(cf[k - 2]))
            if np.imag(cf[k]) == 0:
                cf[k] += 1e-10
            cf[k] = np.log(np.abs(cf[k])) / 2 + cf[k] * 1j
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_783(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.abs(t1 * t2) * np.exp(1j * np.angle(t1 + t2))
        cf[1] = np.angle(t1 * t2) * np.exp(1j * np.abs(t1 - t2))
        for k in range(2, 25):
            cf[k] = np.abs(t1 + t2 * 1j ** (k + 1)) * np.exp(1j * np.angle(cf[k - 1] + t1 * cf[k - 2]))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_784(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.real(t1 + t2) + np.conj(t1 * t2)
        for k in range(1, 25):
            n = np.abs(t1 * cf[k - 1])
            if n != 0:
                cf[k] = np.sin(np.angle(t1)) * np.log(np.abs(n)) + np.cos(np.angle(t2)) * np.log(np.abs(1 / n))
            else:
                cf[k] = 0
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_785(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[:5] = np.array([1, t1, t2, t1 * t2, np.abs(t1 + t2)]) ** 2
        for k in range(5, 25):
            cf[k] = 3 * cf[k - 1] + 2 * cf[k - 5] + 5 * (k + 1)
        cf[9:15] = np.conj(cf[:6])
        cf[19:25] = np.exp(1j * np.angle(cf[:6]))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_786(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            cf[k] = np.abs(t1) ** (k / 2) * (np.cos(k * np.angle(t2)) + 1j * np.sin(k * np.angle(t2)))
        cf[4] += (np.log(np.abs(t1)) + np.log(np.abs(t2))) / 2
        cf[9] += np.conj(t1 * t2)
        cf[14] += np.abs(t2 - t1) ** 2
        cf[19] += (np.sin(np.angle(t1)) / np.cos(np.angle(t2))) ** 3
        cf[24] += ((1j * t1 - t2) ** 2 / (1 + np.abs(t1 + t2) ** 3)) ** 4
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_787(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.real(t1 + t2) + 1j * np.imag(t1 * t2)
        for i in range(1, 24):
            cf[i] = np.abs(cf[i - 1] ** (t1 - t2)) + 1j * np.angle(cf[i - 1])
            if np.isinf(cf[i]) or np.isnan(cf[i]):
                cf[i] = 1j
        cf[24] = cf[0] ** 3 + cf[23] ** 2 + cf[22] - cf[21] + np.conj(cf[20])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_788(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = (t1 + 1j * t2)
        cf[1] = 2 * np.real((t1 + 1j * t2)) ** 2
        cf[2] = 3 * np.imag((t1 + 1j * t2)) ** 3
        for k in range(3, 15):
            cf[k] = np.abs((cf[k - 1] ** (k + 1) + cf[k - 2] ** k)) / (k ** 2 + 1) + (np.angle(t1) + np.angle(t2))
        cf[15:20] = cf[10:15] + cf[:5]
        cf[20] = np.abs(t1) ** 2 - np.abs(t2) ** 2
        cf[21] = np.angle(t1) + np.angle(t2)
        cf[22] = np.real(t1 ** 3 - t2 ** 3)
        cf[23] = np.imag(t1 * t2 * (t1 - t2))
        cf[24] = np.abs(t1 * t2 * (t1 - t2)) ** 0.5
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_789(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[:5] = np.real(t1) * np.arange(1, 6) - np.imag(t2) * np.arange(1, 6)
        cf[5] = np.prod(np.abs(t1), np.abs(t2))
        cf[6:11] = np.angle(t1 + t2) * np.arange(6, 11)
        cf[12] = np.conj(t1) + np.conj(t2)
        cf[13:18] = np.real(t1 + 1j * t2) * np.arange(1, 6)
        cf[18] = np.prod(np.angle(t1), np.angle(t2))
        cf[19:24] = np.imag(t1 - 1j * t2) * np.arange(1, 6)
        cf[24] = np.conj(t1 * t2)
        cf[25] = np.abs(cf[12]) + np.angle(cf[18])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_790(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, 25):
            v = np.sin(k * cf[k - 1] + np.angle(t2 ** k)) + np.cos(k * np.abs(t1))
            cf[k] = v / (np.abs(v) + 1e-10)
        cf[9] = t1 * t2 - np.abs(t2) ** 2 + 1j * np.angle(t1)
        cf[14] = np.conj(t1) ** 3 - np.angle(t2) ** 3 + 1j * np.abs(t2)
        cf[19] = np.abs(t2) ** 3 + t1 ** 2 + t2 ** 2 + 1j * np.angle(t2) ** 2
        cf[24] = np.abs(t1 * t2) + np.angle(t1) ** 5 + 1j * np.abs(t1) ** 5
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_791(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[:10] = np.arange(1, 11) + t1 ** 2 + t2 ** 2
        for i in range(10, 20):
            cf[i] = 2 * cf[i - 1] * np.sin(t1 + t2)
        cf[20:22] = np.array([np.sum(cf[:20]), np.prod(cf[:20])]) * (t1 + t2)
        cf[22] = np.log(np.abs(t1 * t2)) + np.angle(t1 ** 2 + t2 ** 2) * cf[21]
        cf[23] = np.abs(t1 - t2) / (np.abs(cf[22]) + 1)
        cf[24] = np.conj(cf[23] * cf[21])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_792(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.sin(t1 + t2) + 1j * np.cos(t1 - t2)
        for k in range(1, 25):
            v = np.exp(k * cf[k - 1]) + np.log(np.abs(t1)) - np.log(np.abs(t2))
            if not np.isnan(v) and not np.isinf(v):
                cf[k] = v
            else:
                cf[k] = 1 + 1j
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_793(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = np.abs(t1) * t2
        cf[2] = np.conj(t1) + np.real(t2)
        cf[3] = np.abs(t1) * np.imag(t2)
        cf[4] = np.angle(t1) * np.conj(t2)
        for k in range(5, 26):
            cf[k] = np.abs(cf[k - 1] * t1) + np.angle(cf[k - 2] * t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_794(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            cf[k] = np.abs(t1) ** k + np.angle(t2) ** k + t1 * (1j ** k) + np.conj(t2) ** ((k + 1) / 2)
        cf[9] += np.log(np.abs(t1 * t2))
        if not np.isinf(cf[9]) and not np.isnan(cf[9]):
            cf[9] /= np.abs(t1 + t2 + 1j)
        cf[19] += (t1 + 1j * t2) ** 2
        if not np.isinf(cf[19]) and not np.isnan(cf[19]):
            cf[19] /= np.abs(t1 + t2 + 1j) ** 2
        cf[24] = (cf[4] ** 2 + cf[5] ** 2 + cf[6] ** 2) ** 0.5
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_795(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for i in range(25):
            cf[i] = (i ** 2 + t1) * np.exp((1 + 0j) * i * t2) / (1 + np.abs(t1 * t2))
        cf[4:15] = np.real(cf[4:15]) * np.cos(np.imag(cf[4:15]))
        cf[16:25] = np.abs(cf[16:25]) * np.exp(1j * np.angle(t1 + t2))
        cf[2] = (t1 + 1j * t2) ** 3 - np.conj(t1 + 1j * t2) ** 3
        cf[6] = cf[22] = (np.abs(t1) ** 3 + np.abs(t2) ** 3) * np.exp(1j * np.angle(t2 - t1))
        cf[18] = np.where(np.abs(t2) > 1, np.log(np.abs(t2)), 0)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_796(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            phase = np.angle(t1) * np.angle(t2)
            modulus = np.abs(t1 + t2)
            cf[k] = modulus ** (k + 1) * np.exp(1j * phase / (k + 1))
        cf[0] = t1 ** 5 + t2 ** 5
        cf[2] = np.real(t1) + np.imag(t2)
        cf[4] = t1 * t2 * (1 + 1j)
        cf[14] = np.abs(t1 + t2 + 1j) ** 3
        cf[19] = np.conj(t1) + np.real(t2)
        cf[24] = (np.abs(t1) ** 2 + np.abs(t2) ** 2) ** 0.5
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_797(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = 1 + t1 * t2
        cf[1] = -(t1 + t2) - 1j
        for k in range(2, 25):
            cf[k] = cf[k - 1] * cf[1] / cf[k - 2]
            cf[k] += t2 ** (k + 1) + 1j * t1 ** (k + 1)
            cf[k] *= np.exp(1j * np.angle(cf[k - 2]))
            cf[k] /= np.abs(cf[k])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_798(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[1] = np.abs(t1) * np.sin(np.angle(t2))
        cf[2] = np.abs(t2) * np.cos(np.angle(t1))
        for k in range(3, 26):
            cf[k] = np.abs(cf[k - 1]) * np.sin(np.angle(cf[k - 2])) + t1
            if np.abs(cf[k]) > 10000:
                cf[k] /= np.abs(cf[k])
        cf[0] = cf[24]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_799(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for i in range(25):
            cf[i] = (2 * np.real(t1 * t2) + 3 * np.imag(t1 * t2)) * (np.abs(t1) ** (i + 1)) + (2 * np.angle(t1 * t2) - 3 * np.abs(t2)) * (np.abs(t2) ** (i + 1))
            if not np.isfinite(cf[i]):
                cf[i] = 0
        cf[0] += np.conj(t1 * t2)
        if not np.isfinite(cf[0]):
            cf[0] = 0
        cf[12] *= np.log(np.abs(t1 + t2))
        if not np.isfinite(cf[12]):
            cf[12] = 0
        cf[24] -= cf[24] * t1 * t2 / (t1 + t2)
        if not np.isfinite(cf[24]):
            cf[24] = 0
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_800(t1, t2):
    cf = np.zeros(25, dtype=np.complex128)
    cf[0] = t1 * t2
    cf[1:10] = np.abs(t1) * np.angle(1j * t2) ** np.arange(1, 10)
    cf[14] = np.real(t1) * np.imag(t2) + np.real(t2) * np.imag(t1)
    cf[15:20] = np.abs(t1 - t2 + 1j) ** np.arange(1, 6)
    cf[20] = np.imag(t1) * np.real(t2) + np.real(t1) * np.imag(t2)
    cf[21:24] = np.abs(cf[14] + t1 * t2) ** np.arange(1, 4)
    cf[24] = np.abs(cf[23]) + np.log(np.abs(t1 + t2 + 0.5j))
    return cf.astype(np.complex128)

def poly_801(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.abs(t1) * np.abs(t2)
        cf[1] = np.abs(t1 + t2)
        cf[2] = np.abs(np.conj(t1) + np.conj(t2))
        cf[3] = np.angle(t1) * np.angle(t2)
        cf[4] = np.angle(np.conj(t1) + np.conj(t2))
        cf[5] = np.exp(1j * np.angle(t1 + t2))
        cf[6] = np.exp(1j * np.angle(np.conj(t1) + np.conj(t2)))
        cf[7] = np.sin(np.abs(np.conj(t1)) * np.abs(np.conj(t2)))
        cf[8] = np.cos(np.angle(t1 + t2))
        cf[9] = np.tanh(np.abs(np.conj(t1 + t2)))
        cf[10:15] = np.abs(t1) ** (np.arange(11, 16) / 10) * np.abs(t2) ** (np.arange(15, 10, -1) / 10)
        cf[15:20] = (np.arange(16, 21) * (np.angle(t1) + np.angle(t2))) / 2
        cf[20:25] = np.real(t1) ** 2 + np.imag(t2) ** 2 + np.arange(21, 26)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_802(t1, t2):
    cf = np.zeros(25, dtype=np.complex128)

    # Adjusted for Python's 0-based indexing
    cf[0:3] = [2*t1 + 3*t2, 3*t1 - 2*t2, t1**2 - t2**2]
    cf[3:5] = [np.real(t1 * t2), np.imag(t1 * t2)]

    for k in range(5, 20):  # indices 5 to 19 inclusive (6:20 in R)
        cf[k] = np.sin(cf[k-1]) + np.cos(cf[k-2])
        mod_cf = np.abs(cf[k])
        if mod_cf != 0:
            cf[k] = cf[k] / mod_cf
        else:
            cf[k] = 1
    return cf.astype(np.complex128)

def poly_803(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[1] = np.abs(t1) * np.abs(t2)
        cf[2] = np.real(t1 * t2)
        cf[3] = np.imag(t1 * t2)
        cf[4] = np.angle(t1 + t2)
        for k in range(5, 26):
            cf[k] = cf[k - 1] + np.cos(k * t1) + np.sin(k * np.abs(t2)) * 1j
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_804(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0:10] = np.arange(1, 11) * t1 + np.arange(11, 21) * 1j * t2
        cf[10:20] = (t1 + 1j * t2) ** 2 * np.arange(11, 21)
        cf[20:25] = (np.abs(t1) + np.angle(t2)) * np.arange(1, 6)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_805(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 ** 2 + t2 ** 2
        cf[1] = t1 * t2
        cf[2:8] = t1 * t2 ** np.arange(1, 7)
        cf[8:14] = (t1 ** 2 + t2 ** 2) / np.arange(1, 7)
        cf[14:20] = t1 ** 3 * t2 ** np.arange(1, 7)
        cf[20:25] = (t1 + t2) ** np.arange(1, 6)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_806(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cylotomic_coefs = np.array([np.exp(2j * np.pi / 3), np.exp(-2j * np.pi / 3)])
        for k in range(25):
            cf[k] = np.sin(k * t1) * cylotomic_coefs[0] + np.cos(k * t2) * cylotomic_coefs[1]
            if np.imag(cf[k]) == 0:
                cf[k] /= np.real(cf[k])
            else:
                cf[k] /= np.imag(cf[k])
        cf[7] = t1 ** 2 + t2 ** 2
        cf[24] = np.abs(t1 + t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)


def poly_807(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = t1**2 + 2 * t2**2
        cf[2] = t1**4 - t2**4
        for k in range(3, 14):  # indices shifted by -1
            cf[k] = (t1 * t2) / (k + 1) + (1j * cf[k - 1])
        cf[14] = cf[13].real + cf[9].imag
        for k in range(15, 25):
            arg_val = np.angle(cf[k - 1] + 1j * t2**2)
            cf[k] = np.floor(arg_val)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)



def poly_808(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(25):
            cf[k] = (k + t1) / (k + t2)
        cf[4] += np.log(np.abs(t1 + t2))
        cf[9] += np.sin(np.real(t1)) + np.cos(np.imag(t2))
        cf[14] += np.abs(cf[13]) ** 2 + np.angle(cf[12]) ** 2
        cf[19] += np.abs(np.real(t2) * np.imag(t1))
        cf[24] += np.abs(t1 + np.conj(t2))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_809(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        for k in range(1, 25):
            cf[k] = np.cos(k * np.angle(cf[k - 1])) + np.sin(k * np.abs(t1)) + np.conj(t2) / np.abs(1 + t1)
            if np.isinf(cf[k]) or np.isnan(cf[k]):
                cf[k] = cf[k - 1]
        cf[9] = cf[0] ** 3 + cf[1] ** 2 - cf[0] * cf[1]
        if np.isinf(cf[9]) or np.isnan(cf[9]):
            cf[9] = cf[8]
        cf[14] = np.log(np.abs(cf[13])) - t1 ** 2 + t2 ** 2
        if np.isinf(cf[14]) or np.isnan(cf[14]):
            cf[14] = cf[13]
        cf[19] = cf[0] * (t1 + t2) ** 2 - cf[2] / (1 + np.abs(t1 * t2))
        if np.isinf(cf[19]) or np.isnan(cf[19]):
            cf[19] = cf[18]
        cf[24] = (t1 + t2) ** 3 - cf[23]
        if np.isinf(cf[24]) or np.isnan(cf[24]):
            cf[24] = cf[23]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_810(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for i in range(25):
            if i % 2 == 1:
                cf[i] = ((i * t1 + 3 * i * t2) / (i + 1) ** 2) ** i
            else:
                cf[i] = (t1 + np.conj(t2)) ** i
        cf[cf == np.inf] = 1e10
        cf[cf == -np.inf] = -1e10
        cf[np.isnan(cf)] = 0
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_811(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 * t2
        cf[1] = (t1 + t2) * np.conj(t1 - t2)
        for k in range(2, 26):
            cf[k] = np.abs(t1) * np.abs(t2) * np.sin(np.angle(t1 + 1j * t2) ** k) + np.log(np.abs(t1 ** k / (1 + t2)))
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_812(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 * t2
        for k in range(1, 26):
            cf[k] = (cf[k - 1] ** 2 + np.real(t1) + np.imag(1j * t2)) / (1 + np.abs(cf[k - 1]))
            if np.abs(cf[k]) > 1e6 or np.isnan(cf[k]) or np.isinf(cf[k]):
                cf[k] = cf[k - 1]
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_813(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 * t2
        for k in range(1, 26):
            v = (np.sin(k * cf[k - 1]) + np.cos(k * cf[k - 1])) * np.real(t1 + t2)
            if np.abs(v) != 0:
                cf[k] = np.log(np.abs(v)) + np.conj(t1 * t2)
            else:
                cf[k] = 0
        cf[24] = np.sum(cf[:24]) + np.abs(t1 - t2)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_814(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = np.angle(t1) * t2
        cf[1] = t1 * t2
        for i in range(2, 26):
            v = cf[i - 1] + cf[i - 2] + np.conj(t1 * t2)
            if np.abs(v) != 0:
                cf[i] = np.log(np.abs(v))
            else:
                cf[i] = 0
        cf[24] = np.abs(t1 - t2) + np.sum(cf[:24])
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(0, dtype=np.complex128)

def poly_815(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(1, 26):
            cf[k-1] = np.sin(k)*t1/(1+abs(t2)) + np.cos(k)*t2/(1+abs(t1)) + np.sqrt(k)
        
        cf[0] = np.abs(t1)*abs(t2)
        cf[4] = np.angle(t1)*abs(t2)
        cf[9] = np.abs(t1)*np.angle(t2)
        cf[14] = np.abs(t1)*t2.real
        cf[19] = np.abs(t1)*t2.imag
        cf[24] = t1.real*abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_816(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1*t2
        cf[1] = np.abs(t1)**2 + np.abs(t2)**2
        for k in range(2, 24):
            cf[k] = (cf[k-1] / cf[k-2]) + np.conj(t1) - t2
            if np.isnan(cf[k]) or np.isinf(cf[k]):
                cf[k] = 0
        cf[24] = cf[23] + cf[22] - np.conj(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_817(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = (t1**3).real*(t2**3).real
        cf[1] = (t1**3).imag*(t2**3).imag
        for k in range(2, 25):
            if (k+1)%3 == 0:
                cf[k] = (t1+1j*t2)**((k+1)/3) / (k+1)
            else:
                cf[k] = np.conj(cf[k-1])**2 + np.abs(t1)*abs(t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_818(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        for k in range(1, 26):
            cf[k-1] = (np.abs(t1 + 1j*t2 + 0.5 + k/25)*np.cos(np.angle(t1 + 1j*t2)**(k-1)) + 
                       1j*abs(t2 + 1j*t1 + 0.5 + k/25)*np.sin(np.angle(t2 + 1j*t1)**(k-1)))
            if np.isnan(cf[k-1]) or np.isinf(cf[k-1]):
                cf[k-1] = 0
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_819(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1*t2
        for i in range(1, 25):
            cf[i] = (cf[i-1].real**2 - cf[i-1].imag**2)*t1*t2 + 1j*2*cf[i-1].real*cf[i-1].imag
            if np.isnan(cf[i]):
                cf[i] = 1.0
            if np.abs(cf[i]) < 1e-10:
                cf[i] = 1
        cf[24] = cf[23] + t1 + t2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_820(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = t1 + t2
        cf[4] = 5 * np.real(t1 * t2)
        cf[8] = 9 * np.sin(np.angle(t1 * np.conj(t2)))
        for i in range(1, 4):
            cf[i] = cf[i-1]**2 + cf[4]
        for j in range(5, 8):
            cf[j] = np.abs(t1)**(cf[j-1]) + cf[0]
        for k in range(9, 25):
            cf[k] = np.log(np.abs(cf[k-1])+1) + cf[8]
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_821(t1, t2):
    try:
        cf = np.zeros(25, dtype=np.complex128)
        cf[0] = 3 * t1 + 5j * t2
        for k in range(1, 25):
            mod_t1 = np.abs(t1)
            arg_t2 = np.angle(t2)
            cf[k] = cf[k-1] * (mod_t1 + arg_t2)
            if cf[k].real < 0 and cf[k].imag < 0:
                cf[k] = np.conj(cf[k])
            if np.abs(cf[k].real) > 10:
                cf[k] = cf[k] / mod_t1
            if np.abs(cf[k].imag) > 10:
                cf[k] = cf[k] / (1j * arg_t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

#convert every single one of these functions into python. t1, t2 are complex numbers. wrap the translation into try:....except: ..
#if there is an error return return np.zeros(0, dtype=np.complex128).
#use numpy for all math.
#
def polymoth(t1, t2):
    try:
      cf = np.zeros(35, dtype=np.complex128)  
      cf[0] = t1 + t2
      for k in range(1, 35):
          v = np.sin(k * cf[k-1]) + np.cos(k * t1)
          mag = np.abs(v)
          if mag == 0:
              cf[k] = 0 + 0j
          else:
              cf[k] = v / mag
      return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def polymoth1(t1, t2):
    try:
      cf = np.zeros(90, dtype=np.complex128)  
      cf[0] = t1 - t2
      for k in range(1, 90):
          v = np.sin(k * cf[k-1]) + np.cos(k * t1)
          mag = np.abs(v)
          if mag == 0:
              cf[k] = t1 + t2
          else:
              cf[k] = 1j * v / mag
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

def giga_moth4(t1, t2):
    cf = np.zeros(50, dtype=np.complex128)
    cf[0] = t1 + t2
    for k in range(1, len(cf)):
        v = np.sin(((k + 4) % 10) * cf[k - 1]) + np.cos((k % 10) * t1)
        av = np.abs(v)
        if np.isfinite(av) and av > 1e-10:
            cf[k] = v / av
        else:
            cf[k] = t1 + t2
    return cf.astype(np.complex128)

#
def giga10(t1, t2):
    try:
      n = 120
      cf = [0+0j]*n
      re1 = t1.real
      im1 = t1.imag
      re2 = t2.real
      im2 = t2.imag
  
      for k in range(1, n+1):
          k_idx = k-1  # python index
          term1_mag = 100*(re1 + im2)*((k/10.0)**2)
          term1_ang = (re2 * k)/20.0
          term1 = term1_mag * cmath.exp(1j*term1_ang)
          
          sinval = cmath.sin(k * 0.1 * im2)
          term2_mag = 50*(im1 - re2)*sinval
          term2_ang = -1.0 * k * 0.05 * re1
          term2 = term2_mag * cmath.exp(1j*term2_ang)
          
          cf[k_idx] = term1 + term2
  
      cf[29] += 1000j
      cf[59] -= 500
      cf[89] += 250 * cmath.exp(1j*(t1*t2))
  
      return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
  
#  
def old_379(t1, t2):
    cf = np.zeros(35, dtype=np.complex128)
    for j in range(1, 36):
        magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (
            (np.abs(t1) ** np.sin(j)) + (np.abs(t2) ** cmath.cos(j))
        )
        angle = (
            np.angle(t1) * j
            - np.angle(t2) * (35 - j)
            + np.sin(j) * np.cos(j)
        )
        cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
    for k in range(1, 36):
        cf[k - 1] += t1.conjugate() * (t2 ** k) / (k + 1)
    special_indices = [4, 9, 14, 19, 24, 29]
    for i in special_indices:
        cf[i] += 50 * (t1.real - t2.imag) * 1j
    return cf.astype(np.complex128)
  
#
def old1(t1, t2):
    n = 35
    cf = np.zeros(n, dtype=np.complex128)
    rec = np.linspace(t1.real, t2.real, n)
    imc = np.linspace(t1.imag, t2.imag, n)
    for j in range(n):
        jj = j + 1  # R's j (1-based)
        mag = (np.abs(t1)**jj) / (jj + 1) \
              + (np.abs(t2)**(n - jj)) * cmath.sin(jj) \
              + cmath.log(np.abs(rec[j] + 1j*imc[j]) + 1)
        angle = cmath.phase(t1) * cmath.cos(jj) \
                + cmath.phase(t2) * cmath.sin(jj) \
                + cmath.sin(jj * math.pi / n)
        cf[j] = mag * (cmath.cos(angle) + 1j * cmath.sin(angle))
    for k in range(n):
        kk = k + 1  # R's k (1-based)
        cf[k] = ( cf[k]
                  + np.conjugate(cf[(k + 1) % n]) * cmath.cos(kk)
                  - cf[k].real * cmath.sin(kk) )
    for r in range(n):
        rr = r + 1  # R's r (1-based)
        if rr % 3 == 0:
            cf[r] = cf[r] * (1 + 0.5 * cmath.sin(rr))
        else:
            cf[r] = cf[r] + 0.3 * cf[r].imag * cmath.cos(rr)
    return cf.astype(np.complex128)
  
#
def poly48(t1, t2):
    try:
        cf = np.zeros(90, dtype=np.complex128)
        cf[0] = t1 - t2
        for k in range(1, len(cf)):
            v = np.sin(k * cf[k-1]) + np.cos(k * t1)
            av = np.abs(v)
            if np.isfinite(av) and av > 1e-10:
                cf[k] = 1j * v / av
            else:
                cf[k] = t1 + t2
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
 
#     
def poly51(t1, t2):
    try:
      cf = [0j] * 50  # Initialize complex array of size 50
      cf[0] = t1 + t2
      for k in range(1, len(cf)):
          v = np.sin(((k+3) % 10) * cf[k-1]) + np.cos(((k+1) % 10) * t1)
          av = np.abs(v)
          if np.isfinite(av) and av > 1e-10:
              cf[k] = v / av
          else:
              cf[k] = t1 + t2
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#
def poly70(t1, t2):
  try:
    cf = np.zeros(50, dtype=np.complex128)
    cf[[0, 9, 19, 29, 39, 49]] = [1, 2, -3, 4, -5, 6]
    cf[14] = 100 * (t1**2 + t2**2)
    cf[24] = 50 * (cmath.sin(t1) + 1j * cmath.cos(t2))
    cf[34] = 200 * (t1 * t2) + 1j * (t1**3 - t2**3)
    cf[44] = cmath.exp(1j * (t1 + t2)) + cmath.exp(-1j * (t1 - t2))
    return cf.astype(np.complex128)
  except:
    return np.zeros(0, dtype=np.complex128)
 
# 
def poly72(t1, t2):
    try:
      cf = np.zeros(35, dtype=np.complex128)
      cf[[0, 6, 14, 19, 26, 34]] = [1, -2, 3, -4, 5, -6]
      cf[11] = 50j * np.sin(t1**2 - t2**2)
      cf[17] = 100 * (np.cos(t1) + 1j * np.sin(t2))
      cf[24] = 50 * (t1**3 - t2**3 + 1j * t1 * t2)
      cf[29] = 200 * np.exp(1j * t1) + 50 * np.exp(-1j * t2)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#
def poly76(t1, t2):
    try:
      cf = np.zeros(30, dtype=np.complex128)
      cf[[0, 5, 11, 19]] = [1, 3, -2, 5]
      cf[9] = 100 * t1**3 + 50 * t2**2
      cf[14] = 50j * (t1.real - t2.imag)
      cf[24] = 200 * t1 * (t2 + 1) - 100j * t2
      cf[29] = np.exp(1j * t1) + t2**3
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#
def poly77(t1, t2):
    try:
      cf = np.zeros(40, dtype=np.complex128)
      cf[0] = 1
      cf[4] = 50 * cmath.exp(t1)
      cf[9] = 100 * (t2**2 - 1j * t1)
      cf[19] = 200 * cmath.exp(1j * (t1**2)) - 50 * cmath.exp(-1j * (t2**3))
      cf[29] = 100 * t1 * (t2**2) + 50j * (t1**3)
      cf[39] = cmath.exp(1j * (t1 + t2)) - 50 * cmath.sin((t1 - t2).imag)
      return cf.astype(np.complex128) 
    except:
      return np.zeros(0, dtype=np.complex128)
 
#
def poly78(t1, t2):
    try:
      cf = [0j] * 40
      cf[0], cf[7], cf[15], cf[23], cf[31] = 1, -3, 5, -7, 2
      cf[4] = 50 * (t1**2 - t2**3)
      cf[11] = 100j * (t1**3 + t2)
      cf[19] = np.exp(1j * t1) + np.exp(-1j * t2**2)
      cf[29] = 200 * np.sin(t1.real + t2.imag) - 50 * np.cos((t1 - t2).imag)
      cf[34] = np.exp(1j * t1**3) + t2**2
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#
def poly103(t1, t2):
    try:
      cf = np.zeros(25, dtype=np.complex128)
      for i in range(1, 26):
          real_part = (t1.real ** i)
          imag_part = (t2.imag ** (25 - i))
          denominator = (1 + np.abs(t1 + t2))
          phase_factor = cmath.exp(1j * cmath.phase(t1 + t2))
          cf[i - 1] = ((real_part + imag_part) / denominator) * phase_factor
      cf[2] = 3 * np.conjugate(t1**2 + t2)
      cf[6] = 7 * np.abs(t1 + t2)
      cf[10] = 11 * (t1 / t2 + np.conjugate(t2 / t1))
      cf[16] = 17 * (np.abs(t1) * np.abs(t2)) / (np.abs(t1 + t2) ** 2)
      cf[22] = 23 * (np.conjugate(t1) + t2) / (1 + np.abs(t1 * np.conjugate(t2)))
      cf[24] = 25 * (np.conjugate(t1) + np.conjugate(t2)) / np.abs(t1 * t2)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#    
def poly279(t1, t2):
    try:
      cf = [0j] * 71
      cf[0] = t1**5 + 2*t2
      cf[1] = t1**4 - 3*t2**2 + t1.conjugate()
      cf[2] = 4*t1*t2 + 5*complex(np.sin(t1))
      cf[3] = 6*complex(np.cos(t2)) - 7*t1**2
      cf[4] = 8*t2**3 + 9*t1*t2
      cf[5] = 10*complex(np.sin(t1 + t2)) - 11*t2
      cf[6] = 12*complex(np.cos(t1)) + 13*t1*t2
      cf[7] = 14*t1**3 - 15*t2**2
      cf[8] = 16*t1*t2**2 + 17*complex(np.sin(t2))
      cf[9] = 18*complex(np.cos(t1 + t2)) - 19*t1
      cf[10] = 20*t2**4 + 21*t1**2*t2
      cf[11] = 22*complex(np.sin(t1)) - 23*t2**3
      cf[12] = 24*complex(np.cos(t2)) + 25*t1*t2
      cf[13] = 26*t1**4 - 27*t2**2*t1
      cf[14] = 28*complex(np.sin(t1 + t2)) + 29*t2
      cf[15] = 30*complex(np.cos(t1)) - 31*t1*t2**2
      cf[16] = 32*t2**5 + 33*t1**3
      cf[17] = 34*complex(np.sin(t2)) - 35*t1*t2
      cf[18] = 36*complex(np.cos(t1 + t2)) + 37*t2**3
      cf[19] = 38*t1**5 - 39*t2**2
      cf[20] = 40*complex(np.sin(t1)) + 41*t1**2*t2
      cf[21] = 42*complex(np.cos(t2)) - 43*t2**4
      cf[22] = 44*t1**3*t2 + 45*complex(np.sin(t1 + t2))
      cf[23] = 46*complex(np.cos(t1)) - 47*t2**3*t1
      cf[24] = 48*t2**6 + 49*t1**4
      cf[25] = 50*complex(np.sin(t2)) + 51*t1*t2**2
      cf[26] = 52*complex(np.cos(t1 + t2)) - 53*t2**4
      cf[27] = 54*t1**6 - 55*t2**3
      cf[28] = 56*complex(np.sin(t1)) + 57*t1**3*t2
      cf[29] = 58*complex(np.cos(t2)) - 59*t2**5
      cf[30] = 60*t1**4*t2 + 61*complex(np.sin(t1 + t2))
      cf[31] = 62*complex(np.cos(t1)) + 63*t2**4*t1
      cf[32] = 64*t2**7 + 65*t1**5
      cf[33] = 66*complex(np.sin(t2)) - 67*t1*t2**3
      cf[34] = 68*complex(np.cos(t1 + t2)) + 69*t2**5
      cf[35] = 70*t1**7 - 71*t2**4
      cf[36] = 72*complex(np.sin(t1)) + 73*t1**4*t2
      cf[37] = 74*complex(np.cos(t2)) - 75*t2**6
      cf[38] = 76*t1**5*t2 + 77*complex(np.sin(t1 + t2))
      cf[39] = 78*complex(np.cos(t1)) - 79*t2**5*t1
      cf[40] = 80*t2**8 + 81*t1**6
      cf[41] = 82*complex(np.sin(t2)) + 83*t1*t2**4
      cf[42] = 84*complex(np.cos(t1 + t2)) - 85*t2**6
      cf[43] = 86*t1**8 - 87*t2**5
      cf[44] = 88*complex(np.sin(t1)) + 89*t1**5*t2
      cf[45] = 90*complex(np.cos(t2)) - 91*t2**7
      cf[46] = 92*t1**6*t2 + 93*complex(np.sin(t1 + t2))
      cf[47] = 94*complex(np.cos(t1)) + 95*t2**6*t1
      cf[48] = 96*t2**9 + 97*t1**7
      cf[49] = 98*complex(np.sin(t2)) - 99*t1*t2**5
      cf[50] = 100*complex(np.cos(t1 + t2)) + 101*t2**7
      cf[51] = 102*t1**9 - 103*t2**6
      cf[52] = 104*complex(np.sin(t1)) + 105*t1**6*t2
      cf[53] = 106*complex(np.cos(t2)) - 107*t2**8
      cf[54] = 108*t1**7*t2 + 109*complex(np.sin(t1 + t2))
      cf[55] = 110*complex(np.cos(t1)) + 111*t2**7*t1
      cf[56] = 112*t2**10 + 113*t1**8
      cf[57] = 114*complex(np.sin(t2)) - 115*t1*t2**6
      cf[58] = 116*complex(np.cos(t1 + t2)) + 117*t2**8
      cf[59] = 118*t1**10 - 119*t2**7
      cf[60] = 120*complex(np.sin(t1)) + 121*t1**7*t2
      cf[61] = 122*complex(np.cos(t2)) - 123*t2**9
      cf[62] = 124*t1**8*t2 + 125*complex(np.sin(t1 + t2))
      cf[63] = 126*complex(np.cos(t1)) + 127*t2**8*t1
      cf[64] = 128*t2**11 + 129*t1**9
      cf[65] = 130*complex(np.sin(t2)) - 131*t1*t2**7
      cf[66] = 132*complex(np.cos(t1 + t2)) + 133*t2**9
      cf[67] = 134*t1**11 - 135*t2**8
      cf[68] = 136*complex(np.sin(t1)) + 137*t1**8*t2
      cf[69] = 138*complex(np.cos(t2)) - 139*t2**10
      cf[70] = 140*t1**9*t2 + 141*complex(np.sin(t1 + t2)) / 200
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#
def poly378(t1, t2):
    try:
        cf = np.zeros(35, dtype=np.complex128)
        indices = [0, 4, 12, 19, 20, 24] 
        values = [1 + 0j, 4 + 0j, 4 + 0j, -9 + 0j, -1.9 + 0j, 0.2 + 0j]
        cf[indices] = values
        exclude_indices = set([4, 12, 19, 20, 24]) 
        for j in range(1, 34): 
            if j not in exclude_indices:
                mag = np.log(np.abs(t1 + (j+1)) + 1) * np.sin((j+1) * np.angle(t2)) + np.cos((j+1) * np.angle(t1))
                angle = np.angle(t1)**(j+1) + np.sin((j+1) * np.angle(t2)) - np.cos(j+1)
                cf[j] = mag * np.cos(angle) + mag * np.sin(angle) * 1j
        cf[34] = np.conj(t1) * np.conj(t2) + np.sin(np.abs(t1) * np.abs(t2)) + np.log(np.abs(t1) + np.abs(t2) + 1) * 1j
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
      
#
def poly383(t1, t2):
  try:
      cf = np.zeros(35, dtype=np.complex128)
      fixed_indices = [1, 5, 9, 13, 17, 21, 25, 29, 33]
      fixed_values = [
          2 + 3j,   # 2 + 3i in R
          -3 + 2j,
          4 - 1j,
          -5 + 4j,
          6 - 3j,
          -7 + 5j,
          8 - 4j,
          -9 + 6j,
          10 - 5j
      ]
      cf[fixed_indices] = fixed_values
      j_indices = [1, 5, 9, 13, 17, 21, 25, 29, 33]  # R indices
      j_indices_py = [x - 1 for x in j_indices]
      for j_py in j_indices_py:
          value = (
              (t1.real * t2.imag + t1.imag * t2.real)
              + (np.abs(t1)**2 - np.abs(t2)**2) * cmath.sin(t1 + t2)
              + math.log(np.abs(t1) + 1) * cmath.cos(t2)
          )
          cf[j_py] = value
      k_indices = [3, 7, 11, 15, 19, 23, 27, 31, 35]  # R
      k_indices_py = [x - 1 for x in k_indices]
      for k_py in k_indices_py:
          value = (
              cmath.sin(t1 * t2)
              + cmath.cos(t1 / (np.abs(t2) + 1)) * np.conjugate(t2)
              + cmath.phase(t1 + t2) * np.abs(t1 - t2)
              + (t1.real * t2.imag)
          )
          cf[k_py] = value
      r_indices = [4, 8, 12, 16, 20, 24, 28, 32]  # R
      r_indices_py = [x - 1 for x in r_indices]
      for r_py in r_indices_py:
          value = (
              (t1.real**3)
              - (t2.imag**3)
              + (t1 * t2).real
              + (t1 + t2).imag
              + math.log(np.abs(t1 * t2) + 1)
          )
          cf[r_py] = value
      cf[18] = 100j * (t1**3) + 50j * (t2**2) - 75 * t1 * t2 + 25
      cf[22] = (
          80j * (t2**3)
          - 60j * (t1**2)
          + 40 * cmath.sin(t1 + t2)
          - 20
      )
      cf[26] = (
          90j * (t1 * (t2**2))
          - 70 * cmath.cos(t1)
          + 50 * math.log(np.abs(t2) + 1)
      )
      cf[30] = (
          110j * cmath.sin(t1**2)
          - 95 * np.abs(t2) * t1
          + 85j * cmath.phase(t1 + t2)
      )
      cf[34] = (
          120j * cmath.cos(t1 * t2)
          - 100 * cmath.sin(t2)
          + 75 * math.log(np.abs(t1) + 1)
      )
      return cf.astype(np.complex128)
  except:
      return np.zeros(0, dtype=np.complex128)
    
#  
def poly420(t1, t2):
    try:
      n = 35
      cf = np.zeros(n, dtype=np.complex128)
      special_indices_r = [1, 7, 14, 21, 28, 35]
      special_indices_py = [x - 1 for x in special_indices_r]
      special_values = [2.5, -4.2, 3.8, -16.5, 5.3, 0.6]
      for idx_py, val in zip(special_indices_py, special_values):
          cf[idx_py] = val
      for j in range(2, 35):
          j_py = j - 1
          if j % 4 == 0:
              k = j // 2  # or float(j) / 2
              part1 = 150j * (t1 ** k) + 75 * np.conjugate(t2)
              part2 = cmath.sin(k * cmath.phase(t1))
              cf[j_py] = (part1 * part2) - 50 * math.log(np.abs(t2) + 1)
          elif j % 3 == 0:
              k = j % 5
              re_part = 200 * ((t1 * (t2 ** k)).real)
              im_part = 100j * ((t1 - t2).imag)
              angle_part = math.cos(k * cmath.phase(t2))
              cf[j_py] = (re_part + im_part) * angle_part
          else:
              r = j % 7
              term1 = (np.conjugate(t1) ** r) * (t2 ** j)
              term2 = (np.abs(t1 ** j) * np.abs((t2 ** r)))
              cf[j_py] = term1 + term2
      cf[9] = (180j * (t1 ** 3)
               - 120 * (t2 ** 2)
               + 90 * cmath.sin(t1) * cmath.cos(t2))
      cf[19] = (220j * (t2 ** 4)
                + 130 * (t1 ** 3).real
                - 100 * t2.imag)
      cf[29] = (260j * (t1 ** 2) * t2
                + 160 * math.log(np.abs(t1 * t2) + 1)
                - 110 * np.conjugate(t1))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
 
#
def poly476(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0j] * n
      for k in range(1, n + 1):
          real_part = t1.real * np.cos(k * np.pi / 5) + t2.real * np.sin(k * np.pi / 7)
          imag_part = t1.imag * np.sin(k * np.pi / 6) - t2.imag * np.cos(k * np.pi / 8)
          magnitude = np.sqrt(real_part**2 + imag_part**2) * np.log(np.abs(k) + 1) * (1 + np.sin(k))
          angle = np.arctan2(imag_part, real_part) + np.sin(k * np.angle(t1)) * np.cos(k * np.angle(t2))
          cf[k-1] = magnitude * np.exp(1j * angle)
      for r in range(1, n + 1):
          cf[r-1] = cf[r-1] + np.conj(cf[n - r]) * np.sin(r * np.pi / 10)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#
def poly479(t1, t2):
  try:
    n = ps.poly.get("n") or 35
    cf = np.zeros( n, dtype=np.complex128 )
    for j in range( 1, n + 1 ):
      mag = ( np.abs( t1 ) * cmath.log( j + 1 ) + np.abs( t2 ) * j**0.5 ) / ( 1 + j**1.3 )
      angle = cmath.phase( t1 ) * cmath.sin( j ) + cmath.phase( t2 ) * cmath.cos( j / 2 ) + cmath.sin( j / 3 * cmath.pi )
      perturb = cmath.exp( 1j * ( cmath.sin( j / 4 * pi ) + cmath.cos( j / 5 * pi ) ) )
      cf[j-1] = mag * cmath.exp( 1j * angle ) * perturb
    return cf.astype(np.complex128)
  except:
    return np.zeros(0, dtype=np.complex128)

#
def poly484(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(n, dtype=np.complex128)
      for j in range(1, n + 1):
          rec_part = t1.real * cmath.sin(j / 2.0) + t2.real * cmath.cos(j / 3.0)
          imc_part = t1.imag * cmath.cos(j / 4.0) - t2.imag * cmath.sin(j / 5.0)
          magnitude = (cmath.log(np.abs(rec_part + imc_part) + 1) *
                       (j ** 1.2) *
                       (1 + cmath.sin(j * math.pi / 6.0)))
          angle = (cmath.phase(t1) * cmath.cos(j / 7.0) +
                   cmath.phase(t2) * cmath.sin(j / 8.0))
          cf[j - 1] = magnitude * (cmath.cos(angle) + 1j * cmath.sin(angle))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#
def poly485(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(n, dtype=np.complex128)
      for j in range(1, n + 1):
          mag = 0
          angle = 0
          for k in range(1, (j // 5) + 2):
              mag += np.real(t1) * np.sin(j * k) * np.log(k + 1)
              angle += np.imag(t2) * np.cos(j + k) / (k + 1)
          for r in range(1, 4):
              mag *= (1 + np.real(t1) * 0.1 * r)
              angle += np.angle(t2) * 0.05 * r
          cf[j - 1] = mag * (np.cos(angle) + 1j * np.sin(angle))
      cf *= np.exp(1j * np.sin(np.abs(t1)) * np.arange(1, n + 1))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#
def poly518(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0+0j] * n     
      rec = [t1.real + (t2.real - t1.real) * i/(n-1) for i in range(n)]
      imc = [t1.imag + (t2.imag - t1.imag) * i/(n-1) for i in range(n)]
      
      for j in range(n):
          r = rec[j]
          m = imc[j]
          mag = np.log(np.abs(r**2 + m**2) + 1) * (j + 1)**(np.sin(r) + np.cos(m))
          angle = np.sin(j * r) + np.cos(j * m) + np.angle(t1) * np.sin(m) - np.angle(t2) * np.cos(r)
          cf[j] = mag * np.exp(1j * angle)
      
      return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
      
#    
def poly524(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(n, dtype=np.complex128)
      for j in range(1, n + 1):
          real_part = t1.real * math.sin(j) + t2.real * math.cos(j / 2.0)
          imag_part = t1.imag * math.cos(j) - t2.imag * math.sin(j / 2.0)
          part_magnitude = math.sqrt(real_part**2 + imag_part**2)
          log_part = math.log(j + np.abs(t1) + np.abs(t2))
          magnitude = part_magnitude * log_part
          angle = cmath.phase(t1) * math.sqrt(j) + cmath.phase(t2) * math.cos(j)
          cf[j - 1] = magnitude * (math.cos(angle) + 1j * math.sin(angle))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#  
def poly526(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(n, dtype=np.complex128)
      for j in range(1, n + 1):
          r_part = t1.real * j + t2.real / (j + 1)
          i_part = t1.imag * math.sin(j) + t2.imag * cmath.cos(j)
          mag = cmath.log(np.abs(t1) + j) * ((j % 5) + 1)
          angle = cmath.phase(t1) * cmath.sin(j / 3.0) + cmath.phase(t2) * cmath.cos(j / 4.0)
          cf[j - 1] = (r_part + 1j * i_part) * cmath.exp(1j * angle) * mag
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#  
def poly537(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(35, dtype=np.complex128)
      for j in range(1, 36):  # Mimic R's 1..35
          k = (j % 7) + 1
          r = math.floor(j / 5) + 1
          term1 = (t1.real ** k) * cmath.sin(k * math.pi / 3)
          term2 = (t2.imag ** r) * cmath.cos(r * math.pi / 4)
          magnitude = term1 + term2 + cmath.log(np.abs(t1) + np.abs(t2) + j)
          angle = cmath.phase(t1) * r - cmath.phase(t2) / k + cmath.sin(j) * cmath.cos(j)
          cf[j - 1] = (magnitude * cmath.exp(1j * angle) + np.conjugate(t1 * t2) * ((np.abs(t1) + np.abs(t2)) / j))
          if j % 4 == 0:
              cf[j - 1] *= (cmath.sin(j * math.pi / 5) + cmath.cos(j * math.pi / 6))
          if j % 6 == 0:
              cf[j - 1] += (t1.real ** 2) - (t2.imag ** 2)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
  
#
def poly555(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0j] * n
      r1 = t1.real
      i1 = t1.imag
      r2 = t2.real
      i2 = t2.imag
      for j in range(1, n+1):
          mag = np.log(np.abs(r1 + j) + 1) * (j**1.5 + np.sin(j * r2)) * (1 + np.abs(np.cos(j * i1)))
          ang = np.angle(t1) * np.sin(j * r2) + np.angle(t2) * np.cos(j * i1) + np.sin(j * i2)
          cf[j-1] = mag * (np.cos(ang) + 1j * np.sin(ang))
      return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

# 
def poly562(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0j] * n
      for j in range(1, n+1):
          mag_part1 = t1.real * j**2
          mag_part2 = np.log(np.abs(t2) + j) * np.sin(j * np.angle(t1))
          mag_part3 = np.cos(j * t2.real) * np.sqrt(j)
          magnitude = mag_part1 + mag_part2 + mag_part3
          
          angle_part1 = np.angle(t1) + np.sin(j * t1.real)
          angle_part2 = np.cos(j * t2.imag) - np.angle(t2) / j
          angle = angle_part1 + angle_part2
          
          cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
# 
def poly563(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(n, dtype=np.complex128)
      rec_seq = np.linspace(t1.real, t2.real, n)
      im_seq = np.linspace(t1.imag, t2.imag, n)
      for j in range(1, n + 1):
          angle_part = (cmath.sin(j * math.pi / 6.0) *
                        cmath.cos(j * math.pi / 8.0) +
                        cmath.phase(t1) * cmath.log(j + 1))
          
          magnitude_part = (cmath.log(np.abs(t1) + j**2) * np.abs(cmath.cos(j)) +
                            cmath.log(np.abs(t2) + j) * np.abs(cmath.sin(j / 2.0)))
          
          cf[j - 1] = ((magnitude_part + t1.real * t2.real / (j + 1)) *
                       cmath.exp(1j * angle_part))
          
          if j % 5 == 0:
              cf[j - 1] = cf[j - 1] + np.conjugate(cf[j - 1])
          cf[j - 1] *= (1 + 0.1 * cmath.sin(j))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#
def poly583(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(n, dtype=np.complex128)
      for j in range(1, n + 1):
          k = (j * 5 + 2) % 12 + 1
          r_part = t1.real * np.sin(j * np.pi / k) + t2.real * np.cos(j * np.pi / (k + 1))
          i_part = t1.imag * np.cos(j * np.pi / k) - t2.imag * np.sin(j * np.pi / (k + 1))
          magnitude = np.log(np.abs(t1) + j) * np.abs(np.sin(j * np.pi / 10))
          angle = np.angle(t1) * np.cos(j * np.pi / 8) + np.angle(t2) * np.sin(j * np.pi / 9)
          cf[j - 1] = magnitude * (r_part + 1j * i_part) * (np.cos(angle) + 1j * np.sin(angle))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#
def poly598(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0j] * n
      rec = [x.real for x in np.linspace(t1.real, t2.real, n)]
      imc = [x.real for x in np.linspace(t1.imag, t2.imag, n)]
      for j in range(n):
          phase = (cmath.phase(t1) * (j + 1) + 
                  cmath.phase(t2) / (j + 2) + 
                  math.sin((j + 1) * rec[j]) - 
                  math.cos((j + 1) * imc[j]))
          magnitude = (math.log(np.abs(t1) + np.abs(t2) + j + 1) * 
                      ((j + 1)**2 + math.sin(j + 1) * math.cos(j + 1)))
          cf[j] = magnitude * complex(math.cos(phase), math.sin(phase))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#    
def poly605(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0j] * n  # Initialize array of complex numbers
      for j in range(n):
          mag = 0
          angle = 0
          for k in range(1, j+2):  # j+2 because Python range is exclusive
              mag += np.abs(t1 + k).real * np.sin(k * t2.real)
              angle += (t2**k).imag * np.cos(k / (j+1))
          cf[j] = mag * complex(np.cos(angle), np.sin(angle))
      for j in range(n):
          cf[j] = cf[j] * (1 + 0.05 * (j+1)**2) + cf[j].conjugate() * 0.02
      return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
      
#  
def poly621(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(n, dtype=np.complex128)
      rec1 = t1.real
      imc1 = t1.imag  # Defined in R code but not actually used in part1..part4
      rec2 = t2.real
      imc2 = t2.imag
      for j in range(1, n + 1):
          part1 = (rec1 ** j) * cmath.sin(j * cmath.phase(t2))
          part2 = (imc2 ** (n - j)) * cmath.cos(j * np.abs(t1))
          part3 = cmath.log(np.abs(t1) + np.abs(t2) + j)
          part4 = ((rec1 + j) * (imc2 + j) * cmath.log(np.abs(t1) + 1))
          magnitude = part1 * part2 + part3 * part4
          angle = (cmath.phase(t1) * cmath.sin(j)
                   + cmath.phase(t2) * cmath.cos(j)
                   + cmath.log(np.abs(t1) + 1) / j)
          cf[j - 1] = magnitude * cmath.exp(1j * angle)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
 
#
def poly637(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0j] * n
      for j in range(1, n+1):
          mag_part1 = np.log(np.abs(t1) + j**1.5) * np.sin(j * np.pi / 6)
          mag_part2 = np.abs(t2) / (j + 2) + np.cos(j * np.pi / 4)
          magnitude = mag_part1 + mag_part2 * np.exp(-j / 10)
          
          angle_part1 = np.angle(t1) * np.cos(j / 3)
          angle_part2 = np.angle(t2) * np.sin(j / 5) + np.sin(j**2 / 7)
          angle = angle_part1 + angle_part2
          
          cf[j-1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
  
#
def poly657(t1, t2):
    try:
        n = ps.poly.get("n") or 35
        cf = np.zeros(n, dtype=np.complex128)
        rec_seq = np.linspace(t1.real, t2.real, n)
        imc_seq = np.linspace(t1.imag, t2.imag, n)
        
        for j in range(n):
            mag = np.log(np.abs(t1) + j + 1) * np.abs(np.sin((j + 1) * np.pi / 7)) + np.sqrt(j + 1) * np.cos((j + 1) * np.angle(t2))
            angle = np.angle(t1) * np.sin(j + 1) + np.angle(t2) * np.cos((j + 1) / 3)
            cf[j] = mag * (np.cos(angle) + 1j * np.sin(angle))
            
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


#    
def poly662(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0j] * n
      for j in range(1, n+1):
          phase = (np.sin(j * np.angle(t1)) + 
                  np.cos(j * np.angle(t2)) + 
                  np.log(np.abs(t1) + np.abs(t2) + j))
          magnitude = ((j**2 + np.sqrt(j)) * np.abs(np.sin(j / 3)) + 
                  np.exp(-j / 10) * np.abs(t1 + t2))
          cf[j-1] = magnitude * (np.cos(phase) + np.sin(phase) * 1j)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#     
def poly663(t1, t2):
    try:
        n = ps.poly.get("n") or 40
        cf = np.zeros(n, dtype=np.complex128)
        for j in range(1, n+1):
            mag_part = (np.log(np.abs(t1) + 1) * (j**t2.real) + 
                       sum(range(1, j+1)) * np.sqrt(j))
            angle_part = (np.angle(t1) * np.sin(j) + 
                         np.angle(t2) * np.cos(j) + 
                         np.sin(j * t1.imag) * np.cos(j * t2.imag))
            coeff = mag_part * np.exp(1j * angle_part)
            for k in range(1, 4):
                coeff += ((t1.real**k) * (t2.imag**k) * 
                         np.sin(k * j) / (k + 1))
            cf[j-1] = coeff + np.conjugate(t2) * t1**((j % 5) + 1)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

#   
def poly669(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = [0j] * n
      rec1 = t1.real
      imc1 = t1.imag
      rec2 = t2.real
      imc2 = t2.imag
      for r in range(1, n + 1):
          if r % 3 == 1:
              mag = (np.log(np.abs(t1 + r) + 1) * np.sin(r / n * np.pi) + 
                    np.cos(r * np.pi / 4))
              ang = np.angle(t1) + np.sin(r * np.pi / 6) * np.angle(t2)
          elif r % 3 == 2:
              mag = (np.log(np.abs(t2 + r) + 1) * np.cos(r / n * np.pi) + 
                    np.sin(r * np.pi / 3))
              ang = np.angle(t2) + np.cos(r * np.pi / 5) * np.angle(t1)
          else:
              mag = (np.log(np.abs(t1 * t2 + r) + 1) * np.sin(r / (2 * n) * np.pi) + 
                    np.cos(r * np.pi / 2))
              ang = np.angle(t1 * t2) + np.sin(r * np.pi / 4) * np.cos(r * np.pi / 3)
          cf[r-1] = mag * np.exp(1j * ang)
      for k in range(1, n + 1):
          if k <= n / 3:
              cf[k-1] *= k
          elif k <= 2 * n / 3:
              cf[k-1] *= -k
          else:
              cf[k-1] *= 1 / k
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#  
def poly679(t1, t2):
    try:
      n = ps.poly.get("n") or 35
      cf = np.zeros(n, dtype=np.complex128)
      for j in range(1, n + 1):
          mag_part1 = cmath.log( np.abs(t1) + j ) * cmath.sin(0.3 * j * t2.real)
          mag_part2 = cmath.log(np.abs(t2) + j )  * cmath.cos(0.2 * j * t1.imag)
          mag = mag_part1 + mag_part2
          angle_part1 = cmath.phase(t1) + j * 0.1 * math.pi * cmath.sin( j / 5 )
          angle_part2 = cmath.phase(t2) + j * 0.1 * math.pi * cmath.cos( j / 3 )
          angle = angle_part1 + angle_part2
          cf[j - 1] = mag * ( cmath.cos(angle) + 1j * cmath.sin(angle) )
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#
def poly694(t1, t2):
    try:
        n = ps.poly.get("n") or 35
        cf = np.zeros(n, dtype=np.complex128)
        rec = np.linspace(t1.real, t2.real, n)
        imc = np.linspace(t1.imag, t2.imag, n)
        for j in range(n):
            angle = np.sin((j+1) * rec[j]) + np.cos((j+1) * imc[j]) + np.angle(t1 * np.conj(t2))
            magnitude = np.log(np.abs(rec[j]**2 + imc[j]**2) + 1) * ((j+1)**1.5 + np.prod(rec[:(j+1)] + imc[:(j+1)]))
            cf[j] = magnitude * (np.cos(angle) + np.sin(angle) * 1j)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
      
#   
def poly743(t1, t2):
    try:
        n = ps.poly.get("n") or 25
        degree = n
        cf = np.zeros(degree + 1, dtype=np.complex128)
        for j in range(1, degree + 2):
            mag_part1 = np.log(np.abs(t1) + j) * np.sin(2 * np.pi * t1.real / (j + 1))
            mag_part2 = np.log(np.abs(t2) + j) * np.cos(2 * np.pi * t2.imag / (j + 1))
            magnitude = mag_part1 + mag_part2 + np.prod([t1.real, t2.imag, j])
            angle = np.angle(t1) * j + np.angle(t2) * (degree + 1 - j) + np.sin(j) - np.cos(j)
            cf[j-1] = magnitude * np.exp(1j * angle)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)
      
#
def poly751(t1, t2):
    try:
      n = ps.poly.get("n") or 25
      cf = [0j] * n
      rec = [x.real for x in np.linspace(t1.real, t2.real, n)]
      imc = [x.real for x in np.linspace(t1.imag, t2.imag, n)]
      
      for j in range(n):
          mag = np.log(np.abs(rec[j] + imc[j]) + 1) * (pow(j+1, 2) + np.sin(j+1))
          ang = np.sin(rec[j] * (j+1)) + np.cos(imc[j] * (j+1))
          cf[j] = mag * np.exp(1j * ang) + np.conj(t1) * pow(t2, j+1)
      
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#    
def poly765(t1, t2):
    try:
      n = ps.poly.get("n") or 9
      cf = np.zeros(9, dtype=np.complex128)
      for k in range(1, len(cf)+1):
          mag_part1 = math.log(np.abs(t1) * k + 1)
          mag_part2 = np.abs(math.sin(k * t2.real)) + np.abs(math.cos(k * t1.imag))
          product_val = t1.real * t2.imag * k
          magnitude = mag_part1 * mag_part2 + product_val
          angle = cmath.phase(t1) + cmath.phase(t2) * math.sin(k) + math.cos(k)
          cf[k - 1] = (magnitude * (math.cos(angle) + 1j * math.sin(angle))
                       + np.conjugate(t1) * t2.real)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#  
def poly865(t1, t2):
  try:
    n = ps.poly.get("n") or 9
    cf = np.zeros(n, dtype=np.complex128)
    rec = np.linspace(t1.real, t2.real, n)
    imc = np.linspace(t1.imag, t2.imag, n)
    for k in range(1, n + 1):
        mag = math.log(np.abs(t1) + np.abs(t2) + k) * (k**2)
        angle = (cmath.phase(t1) * math.sin(k)
                 + cmath.phase(t2) * math.cos(k))
        cf[k - 1] = mag * (math.cos(angle) + 1j * math.sin(angle))
    return cf.astype(np.complex128)
  except:
    return np.zeros(0, dtype=np.complex128)
  
#
def poly882(t1, t2):
    try:
      cf = [0j] * 11
      cf[0] = t1**3 + t2**3
      cf[1] = 11 * (t1 + t2)**9
      cf[2] = 1j
      cf[3] = cmath.exp(1j * t1)
      cf[4] = 100 * cmath.sin(t2)
      cf[5] = t1.real - 1j * t2.imag
      cf[6] = 11j * (t2.real / np.abs(t1.imag + 0.1))
      cf[7] = t1 / (np.abs(t1 + t2) + 0.125)
      cf[8] = cmath.exp(1j * t1 * t2)
      cf[9] = np.abs(t1 * t2) * cmath.exp(1j * (cmath.phase(t1) - cmath.phase(t2)))
      cf[10] = t1.real * t2.imag + 10j
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#
def poly884(t1, t2):
  try:
    cf = np.zeros(11, dtype=np.complex128)
    coeff_sequence = np.linspace(
        start=-1 + 2*t1.real,
        stop=1 - 2*t2.imag,
        num=11
    )
    log_factor = (math.log(1 + np.abs(t2)))**2
    for i in range(1, 12):
        seq_sum = np.sum(coeff_sequence[:i])
        cf[i-1] = cmath.exp(1j * i * t1) + seq_sum * log_factor
    cf[10] = cf[10] + cmath.sqrt(cf[0] * cf[1] * t1)
    cf[0] = cf[0] - cmath.sqrt(cf[9] * cf[10] * t2)
    cf[5] = np.sum(cf) / 11
    cf[2] = cf[2] * cf[7] / cf[5]
    cf[7] = cf[7]**2 - cf[4] + cf[8]
    return cf.astype(np.complex128)
  except:
    return np.zeros(0, dtype=np.complex128)
  
#  
def poly901(t1, t2):
    try:
      cf = np.zeros(10, dtype=np.complex128)
      cf[0] = (t1**2 + t2**2) * 1j
      cf[1] = 10
      cf[4] = np.abs(t1 * 100) - 0.5
      cf[5] = np.abs(t2 * 100) - 0.5
      cf[7] = -10
      cf[9] = t1**2 + t2**2
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

# 
def poly907(t1, t2):
    try:
      n = 10
      cf = np.zeros(n, dtype=np.complex128)
      cf[0] = 100 * (cmath.sin(t1)**3) * (cmath.cos(t2)**2)
      cf[1] = 100 * cmath.exp(1j * (t1 + t2)) - 10 * ((t1 - t2)**2)
      cf[2] = (t1 * t2 * (t1 - t2)) / (np.abs(t1) + np.abs(t2) + 1)
      cf[4] = (t1 * t2 * cmath.exp(1j * (t1**2 - t2**2)))**3
      cf[6] = (cmath.sqrt(np.abs(t1)) -
               cmath.sqrt(np.abs(t2)) +
               1j * cmath.sin(t1 * t2))
      cf[7] = 50 * np.abs(t1 - t2) * cmath.exp(1j * np.abs(t1 + t2))
      if t1.imag > 0:
          cf[8] = t1 - np.abs(t2)
      else:
          cf[8] = t2 - np.abs(t1)
      cf[9] = (1j * (t1 * t2))**(0.1 * t1 * t2)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)
    
#  
def poly918(t1, t2):
    try:
      cf = np.zeros(25, dtype=np.complex128)
      cf[0] = t1**2 + t2**2 - t1 * t2
      for k in range(1, 25):
          cf[k] = cf[k - 1] * (t1 + t2) / (1 + np.abs(cf[k - 1]))
      add_indices = [2, 5, 8, 11, 14, 17, 20, 23]
      sub_indices = [1, 4, 7, 10, 13, 16, 19, 22, 24]
      for idx in add_indices:
          cf[idx] += (t1 + 1j * t2)
      for idx in sub_indices:
          cf[idx] -= (t2 + 1j * t1)
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#
def poly958(t1, t2):  
    try:
      cf = [0+0j] * 25
      cf[0] = t1 + t2
      cf[4] = 5 * (t1 * t2).real
      cf[8] = 9 * np.sin(np.angle(t1 * t2.conjugate()))
      for i in range(1, 4):
          cf[i] = cf[i-1]**2 + cf[4]
      for j in range(5, 8):
          cf[j] = np.abs(t1)**(cf[j-1]) + cf[0]
      for k in range(9, 25):
          cf[k] = np.log(np.abs(cf[k-1])+1) + cf[8]
      return cf.astype(np.complex128)
    except:
      return np.zeros(0, dtype=np.complex128)

#  
def poly1000(t1, t2):
    try:
        cf = [0j] * 71
        cf[0] = t2.real**5 + 2*t1.imag
        cf[1] = t2.imag**4 - 3*t1.real**2 + t2.conjugate()
        cf[2] = 4*t2*t1.imag + 5*complex(np.tanh(t2))
        cf[3] = 6*complex(np.cosh(t1)) - 7*t2.real**2
        cf[4] = 8*t1.imag**3 + 9*t2.real*t1
        cf[5] = 10*complex(np.sinh(t2 + t1)) - 11*t1
        cf[6] = 12*complex(np.exp(t2)) + 13*t2.imag*t1
        cf[7] = 14*t2.real**3 - 15*t1.imag**2
        cf[8] = 16*t2*t1.real**2 + 17*complex(np.tanh(t1))
        cf[9] = 18*complex(np.cosh(t2 + t1)) - 19*t2
        cf[10] = 20*t1.real**4 + 21*t2.imag**2*t1
        cf[11] = 22*complex(np.sinh(t2)) - 23*t1.real**3
        cf[12] = 24*complex(np.exp(t1)) + 25*t2.imag*t1
        cf[13] = 26*t2.real**4 - 27*t1.imag**2*t2
        cf[14] = 28*complex(np.tanh(t2 + t1)) + 29*t1
        cf[15] = 30*complex(np.cosh(t2)) - 31*t2*t1.real**2
        cf[16] = 32*t1.imag**5 + 33*t2.real**3
        cf[17] = 34*complex(np.sinh(t1)) - 35*t2.imag*t1
        cf[18] = 36*complex(np.exp(t2 + t1)) + 37*t1.real**3
        cf[19] = 38*t2.imag**5 - 39*t1.real**2
        cf[20] = 40*complex(np.tanh(t2)) + 41*t2.real**2*t1
        cf[21] = 42*complex(np.cosh(t1)) - 43*t1.imag**4
        cf[22] = 44*t2.real**3*t1 + 45*complex(np.sinh(t2 + t1))
        cf[23] = 46*complex(np.exp(t2)) - 47*t1.real**3*t2
        cf[24] = 48*t1.imag**6 + 49*t2.real**4
        cf[25] = 50*complex(np.tanh(t1)) + 51*t2*t1.real**2
        cf[26] = 52*complex(np.cosh(t2 + t1)) - 53*t1.imag**4
        cf[27] = 54*t2.real**6 - 55*t1.real**3
        cf[28] = 56*complex(np.sinh(t2)) + 57*t2.imag**3*t1
        cf[29] = 58*complex(np.exp(t1)) - 59*t1.imag**5
        cf[30] = 60*t2.real**4*t1 + 61*complex(np.tanh(t2 + t1))
        cf[31] = 62*complex(np.cosh(t2)) + 63*t1.imag**4*t2
        cf[32] = 64*t1.real**7 + 65*t2.imag**5
        cf[33] = 66*complex(np.sinh(t1)) - 67*t2*t1.real**3
        cf[34] = 68*complex(np.exp(t2 + t1)) + 69*t1.imag**5
        cf[35] = 70*t2.real**7 - 71*t1.real**4
        cf[36] = 72*complex(np.tanh(t2)) + 73*t2.imag**4*t1
        cf[37] = 74*complex(np.cosh(t1)) - 75*t1.real**6
        cf[38] = 76*t2.imag**5*t1 + 77*complex(np.sinh(t2 + t1))
        cf[39] = 78*complex(np.exp(t2)) - 79*t1.imag**5*t2
        cf[40] = 80*t1.real**8 + 81*t2.real**6
        cf[41] = 82*complex(np.tanh(t1)) + 83*t2*t1.imag**4
        cf[42] = 84*complex(np.cosh(t2 + t1)) - 85*t1.real**6
        cf[43] = 86*t2.imag**8 - 87*t1.imag**5
        cf[44] = 88*complex(np.sinh(t2)) + 89*t2.real**5*t1
        cf[45] = 90*complex(np.exp(t1)) - 91*t1.real**7
        cf[46] = 92*t2.imag**6*t1 + 93*complex(np.tanh(t2 + t1))
        cf[47] = 94*complex(np.cosh(t2)) + 95*t1.real**6*t2
        cf[48] = 96*t1.imag**9 + 97*t2.real**7
        cf[49] = 98*complex(np.sinh(t1)) - 99*t2*t1.imag**5
        cf[50] = 100*complex(np.exp(t2 + t1)) + 101*t1.real**7
        cf[51] = 102*t2.imag**9 - 103*t1.imag**6
        cf[52] = 104*complex(np.tanh(t2)) + 105*t2.real**6*t1
        cf[53] = 106*complex(np.cosh(t1)) - 107*t1.real**8
        cf[54] = 108*t2.imag**7*t1 + 109*complex(np.sinh(t2 + t1))
        cf[55] = 110*complex(np.exp(t2)) + 111*t1.real**7*t2
        cf[56] = 112*t1.imag**10 + 113*t2.real**8
        cf[57] = 114*complex(np.tanh(t1)) - 115*t2*t1.real**6
        cf[58] = 116*complex(np.cosh(t2 + t1)) + 117*t1.imag**8
        cf[59] = 118*t2.real**10 - 119*t1.real**7
        cf[60] = 120*complex(np.sinh(t2)) + 121*t2.imag**7*t1
        cf[61] = 122*complex(np.exp(t1)) - 123*t1.imag**9
        cf[62] = 124*t2.real**8*t1 + 125*complex(np.tanh(t2 + t1))
        cf[63] = 126*complex(np.cosh(t2)) + 127*t1.real**8*t2
        cf[64] = 128*t1.imag**11 + 129*t2.imag**9
        cf[65] = 130*complex(np.sinh(t1)) - 131*t2*t1.real**7
        cf[66] = 132*complex(np.exp(t2 + t1)) + 133*t1.imag**9
        cf[67] = 134*t2.real**11 - 135*t1.real**8
        cf[68] = 136*complex(np.tanh(t2)) + 137*t2.imag**8*t1
        cf[69] = 138*complex(np.cosh(t1)) - 139*t1.real**10
        cf[70] = 140*t2.real**9*t1 + 141*complex(np.sinh(t2 + t1)) / 200
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)


def tst1(t1, t2):
    cf = np.array([
        1,
        t1,
        t2
    ], dtype=np.complex128)
    return cf.astype(np.complex128)

def tst2(t1, t2):
    cf = np.array([
        t2,
        t1,
        1
    ], dtype=np.complex128)
    return cf.astype(np.complex128)

def tst3(t1, t2):
    frame_count = 23
    frame_speed = 1600/8
    frame_start = -800
    time = int(np.abs(t1) * 883) % frame_count
    frame = frame_start + frame_speed * time
    cf = np.array([
        100/(2+t1-t2),
        1/(1+t1),
        - frame / (2 + t1**2 - t2**2 + t1 + t2),
        1/(1+t2),
        100/(1+t1*t2)
    ], dtype=np.complex128)
    return cf.astype(np.complex128)

# Littlewood
def ltlwd(t1,t2):
    n = ps.poly.get("n") or 24
    cf = np.random.choice([-1, 1], size = n).astype(complex)
    return cf.astype(np.complex128)

kabvec=np.array([],dtype=np.complex128)

def kabalistic_vector(s):
    kabala_values = {
        'A': 1, 'B': 2, 'C': 3, 'D': 4, 'E': 5, 'F': 6, 'G': 7, 'H': 8, 'I': 9,
        'J': 10, 'K': 20, 'L': 30, 'M': 40, 'N': 50, 'O': 60, 'P': 70, 'Q': 80,
        'R': 90, 'S': 100, 'T': 200, 'U': 300, 'V': 400, 'W': 500, 'X': 600,
        'Y': 700, 'Z': 800

    }

    
    s = s.replace("0","nulla")
    s = s.replace("1","aleph")
    s = s.replace("2","bet")
    s = s.replace("3","gimel")
    s = s.replace("4","dalet")
    s = s.replace("5","he")
    s = s.replace("6","vav")
    s = s.replace("7","zayin")
    s = s.replace("8","het")
    s = s.replace("9","tet")
    s = s.upper()

    values = [kabala_values.get(char, 0) for char in s]+[1]

    return np.array(values)

def kabala(t1,t2):
    global kabvec
    i = ps.poly.get("i") or 0
    if i==0:
        word = ps.poly.get("word") or "schlong"
        kabvec=kabalistic_vector(word)
    cf = kabvec * t1 - 1j * np.flip(kabvec) * t2
    return cf.astype(np.complex128)
    
cf0 = np.array([],dtype=np.complex128)
def kabala1(t1,t2):
    global cf0
    i = ps.poly.get("i") or 0
    if i==0:
        cf0 = kabala(t1,t2)
    cf1 = kabala(t1,t2)
    cf2 =  ( cf0 + cf1 )
    cf = cf2 / np.abs(cf2) 
    cf0 = cf
    return cf.astype(np.complex128)

def kabala2(t1,t2):
    global cf0
    i = ps.poly.get("i") or 0
    if i==0:
        cf0 = kabala(t1,t2)
    cf1 = kabala(t1,t2)
    cf2 =  ( cf0 + cf1 )
    cf = cf2 / np.abs(cf2) 
    cf0 = cf
    return cf.astype(np.complex128)

# Littlewood deg 24
def ltlwd24(t1,t2):
    n = ps.poly.get("n") or 24
    cf = np.random.choice([-1, 1], size = 24).astype(complex)
    return cf.astype(np.complex128)

def p01d24(t1,t2):
    n = ps.poly.get("n") or 24
    cf = np.random.choice([0, 1], size = n).astype(complex)
    return cf.astype(np.complex128)

def p01d31(t1,t2):
    cf = np.random.choice([0, 1], size = 31).astype(complex)
    return cf.astype(np.complex128)

# Littlewood deg 71
def ltlwd71(t1,t2):
    n = ps.poly.get("n") or 71
    m = ps.poly.get("m") or 11
    degree = np.random.randint(m, n)
    cf0 = np.random.choice([-1, 1], size = degree).astype(complex)
    cf1 = np.arange(degree)+1
    cf = cf0*(cf1**5)
    return cf.astype(np.complex128)    

def prdhd31(t1,t2):
    degree = np.random.randint(3, 31)
    height = np.random.randint(1, 10)
    cf = np.random.choice([-height*t1, height*t2], size = degree).astype(complex)
    return cf.astype(np.complex128)     

def poly_creative3(t1, t2):
    """
    Roots placed on a Lissajous curve parameterized by t1/t2.
    for -x, try none, z01, uc
    """
    try:
        m = ps.poly.get("m") or 250
        n = ps.poly.get("n") or 70
        f1 = int(ps.poly.get("f1") or 0)
        f2 = int(ps.poly.get("f2") or 0)
        f3 = int(ps.poly.get("f3") or 0)
        f4 = int(ps.poly.get("f4") or 0)
        roots = []
        a, b = np.abs(t1), np.abs(t2)
        delta = cmath.phase(t1*t2)
        for k in range(n):
            t = 2*np.pi*k/n
            x = a*np.sin(t + delta)
            y = b*np.sin(2*t + delta)
            roots.append(x + 1j*y)
        coeffs = np.poly(roots).astype(complex)
        k = len(coeffs)
        if f1==1:
            coeffs = coeffs + np.exp(-m)
        if f2==1:
            adj =  np.exp(-m)*np.exp( 1j * 2 * np.pi * t1 ) 
            coeffs = coeffs + adj
        if f3==1:
            adj = np.exp(-m)*np.exp( 1j * 2 * np.pi * t2 )
            coeffs = coeffs + adj
        if f4 == 1 :
            coeffs = coeffs + m*t1
        elif f4 == 2 :
            coeffs = coeffs + t1 + 1j*t2
        elif f4 == 3 :
            coeffs = coeffs + np.exp(-m) * t1 * np.exp( 1j * 2 * np.pi * t2 ) 
        elif f4 == 4 :
            adj = np.exp(-m) * t1 * np.exp( 1j * 2 * np.pi * t2 )
            coeffs = coeffs + np.arange(1,k+1) * adj
        return coeffs
    except Exception as e:
        print(f"Exception message: {e}")
        return np.zeros(n, dtype=np.complex128)
    

def poly_creative4(t1, t2):
    """Modular arithmetic and phase twisting with dynamic shifts."""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        mod1 = int(np.abs(t1)*100) %13 +1
        mod2 = int(np.abs(t2)*100) %17 +1
        for k in range(n):
            freq = (k%mod1)*t1.real + (k%mod2)*t2.imag
            phase = (k*cmath.phase(t1))/(k+1) + (k*cmath.phase(t2))/(k+1)
            cf[k] = (np.sin(freq) + 1j*np.cos(freq)) * cmath.exp(1j*phase)
            cf[k] *= (np.abs(t1)**(k/mod1) + np.abs(t2)**(k/mod2))
        shift = int(t1.imag*10) %n
        return np.roll(cf, shift)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def poly_creative5(t1, t2):
    """Chaotic logistic map dynamics driven by t1/t2."""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        x = 0.5*(t1.real + t1.imag)/(np.abs(t1)+1e-6)
        r = 3.7 + 0.3*(t2.real - t2.imag)/(np.abs(t2)+1e-6)
        for k in range(n):
            x = r*x*(1-x)
            angle = 2*np.pi*x
            cf[k] = x*(np.cos(angle) + 1j*np.sin(angle))
        cf /= np.max(np.abs(cf)) + 1e-6
        return cf.astype(np.complex128)*100
    except:
        return np.zeros(0, dtype=np.complex128)
    
def poly_creative6(t1, t2):
    """Quantum-inspired superposition with entanglement."""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(n):
            state1 = cmath.exp(1j*(k*cmath.phase(t1) + np.abs(t1)))
            state2 = cmath.exp(-1j*(k*cmath.phase(t2) - np.abs(t2)))
            cf[k] = (state1 + state2)/2
            if k > 0:
                cf[k] += cf[k-1]*(t1.real + 1j*t2.imag)
        global_phase = np.sum(cf)/(n+1)
        return cf.astype(np.complex128) * cmath.exp(1j*cmath.phase(global_phase))
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_creative7(t1, t2):
    """Mandelbrot-inspired iterations for fractal patterns."""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(n):
            z, c = t1, t2
            for _ in range(n):
                z = z**4 + c
            cf[k] = z/(k+1)
        return cf.astype(np.complex128) * np.exp(-abs(cf))
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_creative8(t1, t2):
    """Hamiltonian-like terms with position/momentum mixing."""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(n):
            if k%2 ==0:
                q = (k//2+1)*t1.real
                cf[k] = q**2 + 1j*q*t2.imag
            else:
                p = (k//2+1)*t1.imag
                cf[k] = p**2 - 1j*p*t2.real
        cf[::2] += cf[1::2].conj()
        cf[1::2] -= cf[::2].conj()
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_creative9(t1, t2):
    """Fourier series with frequency decay and neighbor mixing."""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(n):
            freq_t1 = (k+1)*cmath.phase(t1)
            freq_t2 = (k+1)*cmath.phase(t2)
            cf[k] = (np.sin(freq_t1)+1j*np.cos(freq_t2)) * np.exp(-abs(t1*t2)*k/n)
        for k in range(1, n-1):
            cf[k] = (cf[k-1] + cf[k+1])*0.5*(t1 + t2)
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_creative10(t1, t2):
    """Geometric algebra product terms with alternating signs."""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(n):
            a = [t1.real, t1.imag]
            b = [t2.real, t2.imag]
            dot = a[0]*b[0] + a[1]*b[1]
            wedge = a[0]*b[1] - a[1]*b[0]
            gp = dot + 1j*wedge
            cf[k] = gp**(k+1)
        cf[::2] *= -1
        return cf.astype(np.complex128)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_neuralnet(t1, t2):
    """Micro neural network with complex-valued activations"""
    try:
        n = ps.poly.get("n") or 71
        # Input layer
        x = np.array([t1.real, t1.imag, t2.real, t2.imag])
        
        # Hidden layer (complex activations)
        w1 = np.array([[0.7, -0.3, 1.2, 0.4],
                      [0.5, 1.1, -0.9, 0.2],
                      [0.3, 0.8, 0.6, -1.0]])
        b1 = np.array([0.1, -0.5, 0.7])
        h = np.tanh(w1 @ x + b1) + 1j*np.sin(w1 @ x[::-1] + b1)
        
        # Output layer
        w2 = np.array([[0.4, -0.8, 1.1],
                      [0.3, 0.9, -0.5],
                      [-0.2, 0.6, 0.7]])
        cf = w2 @ h
        
        # Expand to 71 coefficients using Fibonacci phyllotaxis
        phi = (1 + np.sqrt(5))/2
        angles = np.arange(n) * 2*np.pi/phi**2
        return cf.astype(np.complex128) * np.exp(1j*angles) * np.linspace(1,0.5,n)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_neuralnet1(t1, t2):
    """Micro neural network with complex-valued activations"""
    try:
        n = ps.poly.get("n") or 71
        # Input layer
        x = np.array([t1.real, t1.imag, t2.real, t2.imag])
        
        # Hidden layer (complex activations)
        w1 = np.array([[0.7, -0.3, 1.2, 0.4],
                       [0.5, 1.1, -0.9, 0.2],
                       [0.3, 0.8, 0.6, -1.0]])
        b1 = np.array([0.1, -0.5, 0.7])
        h_real = np.tanh(w1 @ x + b1)
        h_imag = np.sin(w1 @ x[::-1] + b1)
        h = h_real + 1j*h_imag
        
        # Output layer
        w2 = np.array([[0.4, -0.8, 1.1],
                       [0.3, 0.9, -0.5],
                       [-0.2, 0.6, 0.7]])
        cf = w2 @ h  # shape (3,)
        
        # Combine the 3 outputs into one amplitude
        cf_single = cf.sum()  # shape ()
        
        # Expand to n=71 coefficients
        phi = (1 + np.sqrt(5))/2
        angles = np.arange(n) * 2*np.pi/phi**2
        radial = np.linspace(1, 0.5, n)
        
        # This now has shape (71,)
        coeffs = cf_single * np.exp(1j*angles) * radial
        return coeffs
    
    except Exception as e:
        print("poly_neuralnet error:", e)
        return np.zeros(0, dtype=np.complex128)
    

def poly_ca(t1, t2):
    """Cellular automaton rule evolution over coefficient indices"""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        
        # Determine CA rule from parameter angles
        rule = int(np.abs(t1*t2) % 256)
        seed = [int(bit) for bit in bin(int(100*abs(t1-t2)))[2:][-8:]]
        ca = seed * (n//8 + 1)
        
        for k in range(n):
            # Update CA according to rule
            new_ca = []
            for i in range(1, len(ca)-1):
                neighborhood = 4*ca[i-1] + 2*ca[i] + ca[i+1]
                new_ca.append((rule >> neighborhood) & 1)
            ca = [0] + new_ca + [0]
            
            # Convert CA state to complex number
            state_num = sum(2**i * bit for i,bit in enumerate(ca[:8]))
            angle = 2*np.pi*state_num/256
            cf[k] = cmath.exp(1j*angle) * (sum(ca)+1)
        
        return cf.astype(np.complex128) * np.geomspace(1, 0.01, n)
    except:
        return np.zeros(0, dtype=np.complex128)
    
def poly_quantum(t1, t2):
    """Quantum walk-inspired coefficients with phase interference"""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        pos = 0
        state = np.array([1.0, 0.0], dtype=np.complex128)
        
        # Coin operator parameters from inputs
        theta = cmath.phase(t1)
        phi = cmath.phase(t2)
        
        for k in range(n):
            # Quantum coin flip
            coin = np.array([[np.cos(theta), np.sin(theta)*cmath.exp(1j*phi)],
                            [np.sin(theta)*cmath.exp(-1j*phi), -np.cos(theta)]])
            state = coin @ state
            
            # Position update and record coefficient
            pos += 1 if np.random.random() < np.abs(state[0])**2 else -1
            cf[k] = state[0] + 1j*state[1]
            
            # Decoherence factor from parameters
            state *= 0.9 + 0.1*abs(t1-t2)/(np.abs(t1)+abs(t2)+1e-6)
        
        return cf.astype(np.complex128) * np.exp(1j*np.linspace(0, 4*np.pi, n))
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_topological(t1, t2):
    """Knot theory-inspired coefficients using Alexander polynomials"""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        winding = int(np.abs(t1.real*t2.imag - t1.imag*t2.real) % 11)
        
        # Generate coefficients based on knot crossings
        for k in range(n):
            sign = (-1)**(k % winding) if winding >0 else 1
            twist = np.sin(k * cmath.phase(t1)) + 1j*np.cos(k * cmath.phase(t2))
            cf[k] = sign * (np.abs(t1)**(k%5) - np.abs(t2)**(k%3)) * twist
        
        # Mirror symmetry for real knots
        cf[n//2:] = cf[n//2-1::-1].conj()
        return cf.astype(np.complex128) * np.exp(-np.linspace(0, 2, n))
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_biomorphic(t1, t2):
    """Lindenmayer systems meets biochemical oscillations"""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        axiom = 'A'
        rules = {
            'A': f"A{int(np.abs(t1))%5}B",
            'B': f"C[{int(np.abs(t2))%3}A]",
            'C': "AC"
        }
        
        # L-system expansion
        s = axiom
        for _ in range(4):
            s = ''.join([rules.get(c,c) for c in s])
        
        # Convert string to coefficients
        depth = 0
        for k, c in enumerate(s[:n]):
            if c == 'A': cf[k] += 0.5+0.2j
            if c == 'B': cf[k] -= 0.3-0.4j
            if c == 'C': cf[k] *= 1.2+0.8j
            if c == '[': depth += 1
            if c == ']': depth -= 1
            cf[k] *= (0.8**depth) * (1 + 0.1j*k)
        
        # Normalize and apply spiral
        return cf.astype(np.complex128) * np.exp(1j*np.linspace(0, 8*np.pi, n))
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_gravitational(t1, t2):
    """N-body simulation in coefficient space"""
    try:
        n = ps.poly.get("n") or 71
        masses = np.abs([t1, t2, t1+t2, t1-t2])
        positions = np.array([t1, t2, t1*t2, t1/t2], dtype=np.complex128)
        
        cf = np.zeros(n, dtype=np.complex128)
        for k in range(n):
            # Calculate gravitational forces
            forces = np.zeros(4, dtype=np.complex128)
            for i in range(4):
                for j in range(4):
                    if i != j:
                        r = positions[j] - positions[i]
                        forces[i] += masses[j]*r/(np.abs(r)**3 + 1e-6)
            
            # Update positions and record
            positions += 0.1*forces
            cf[k] = np.sum(positions * masses[:,None])
            
            # Parameter-dependent damping
            positions *= 0.95 + 0.05*abs(t1-t2)/(np.abs(t1)+abs(t2)+1e-6)
        
        return cf.astype(np.complex128) * np.geomspace(1, 0.001, n)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_sonic(t1, t2):
    """Audio waveform synthesis in complex plane"""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        
        # FM synthesis parameters from inputs
        carrier_freq = 440*(np.abs(t1)/max(np.abs(t1),1))
        mod_freq = 440*(np.abs(t2)/max(np.abs(t2),1))
        mod_index = 10*abs(t1-t2)/(np.abs(t1)+abs(t2)+1e-6)
        
        for k in range(n):
            t = k/n
            # FM synthesis equation
            mod = mod_index * np.sin(2*np.pi*mod_freq*t)
            wave = np.sin(2*np.pi*carrier_freq*t + mod)
            # Convert to analytic signal
            cf[k] = wave + 1j*np.cos(2*np.pi*carrier_freq*t + mod)
        
        # Apply frequency-dependent phase shift
        return cf.astype(np.complex128) * np.exp(1j*np.linspace(0, 4*np.pi, n))
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_cryptic(t1, t2):
    """Number theory meets elliptic curve cryptography"""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        
        # Use inputs to seed prime field operations
        p = 2**256 - 2**32 - 977  # secp256k1 prime
        a = int(np.abs(t1)*1e6) % p
        b = int(np.abs(t2)*1e6) % p
        
        # Elliptic curve point addition
        x, y = a, b
        for k in range(n):
            # EC point doubling formula
            s = (3*x**2 + a) * pow(2*y, p-2, p) % p
            x_new = (s**2 - 2*x) % p
            y_new = (s*(x - x_new) - y) % p
            x, y = x_new, y_new
            
            # Map to complex plane
            cf[k] = complex(x/p, y/p) * (-1)**k
        
        return cf.astype(np.complex128) * np.logspace(0, -3, n, base=np.e)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_holographic(t1, t2):
    """Wave interference patterns with parameterized diffraction"""
    try:
        n = ps.poly.get("n") or 71
        x = np.linspace(-1, 1, n)
        y = np.linspace(-1, 1, n)
        X, Y = np.meshgrid(x, y)
        
        # Dynamic grating parameters from inputs
        freq = 10*(np.abs(t1) + np.abs(t2))
        angle = cmath.phase(t1*t2)
        phase_mod = np.exp(1j*(X*np.cos(angle) + Y*np.sin(angle))*freq)
        
        # Compute holographic pattern
        pattern = np.fft.fftshift(np.fft.fft2(phase_mod)).flatten()
        return pattern[:n] * np.exp(-(x**2 + y**2)/0.5)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_chaosmorph(t1, t2):
    """Chaotic attractor coefficient sampling"""
    try:
        n = ps.poly.get("n") or 71
        cf = np.zeros(n, dtype=np.complex128)
        # Rossler system parameters from inputs
        a = 0.2 + 0.1*abs(t1)
        b = 0.2 + 0.1*abs(t2)
        c = 5.7 + cmath.phase(t1*t2)
        x, y, z = 0.1, 0.0, 0.0
        
        for k in range(n*10):  # Transient steps
            dx = -y - z
            dy = x + a*y
            dz = b + z*(x - c)
            x += 0.01*dx
            y += 0.01*dy
            z += 0.01*dz
            if k >= n*5:  # Record strange attractor points
                cf[k%n] += complex(x,y) * np.exp(-0.1*z)
        
        return cf.astype(np.complex128) * np.geomspace(1, 0.001, n)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_quasicrystal(t1, t2):
    """Aperiodic Penrose-like tiling in coefficient space"""
    try:
        n = ps.poly.get("n") or 71
        phi = (1 + np.sqrt(5))/2
        angles = np.arange(n) * 2*np.pi/phi
        radii = np.sqrt(np.arange(n)/n)
        
        # Projection from 5D space with golden ratio distortions
        cf = np.sum([np.exp(1j*(radii*k*np.cos(angles + t1.real) + 
                               radii*k*np.sin(angles*phi + t2.imag)))
                   for k in [1, phi, phi**2]], axis=0)
        
        # Apply phasonic modulation
        return cf.astype(np.complex128) * np.exp(1j*np.sin(radii*abs(t1*t2)))
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_neuroevolution(t1, t2):
    """Coefficients evolve via genetic algorithm mechanics"""
    try:
        n = ps.poly.get("n") or 71
        population = np.random.rand(10,n) + 1j*np.random.rand(10,n)
        fitness = np.zeros(10)
        
        for generation in range(5):
            # Evaluate fitness using parameter landscape
            for i in range(10):
                roots = np.roots(population[i])
                fitness[i] = -np.sum(np.abs(roots**2 - t1*roots - t2))
            
            # Selection and crossover
            parents = population[np.argsort(fitness)[-2:]]
            children = np.zeros((8,n), dtype=np.complex128)
            for c in range(8):
                cross = np.random.randint(0,n,size=n)
                children[c] = np.where(np.random.rand(n)<0.5, 
                                     parents[0], parents[1])
            
            # Mutation and elite preservation
            population = np.vstack([parents, 
                                   children + 0.1*np.random.randn(8,n)])
        
        return population[0] * np.hanning(n)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_fluid(t1, t2):
    """Navier-Stokes inspired vorticity transport"""
    try:
        n = ps.poly.get("n") or 71
        # Initialize velocity field
        vx = np.zeros((n,n))
        vy = np.zeros((n,n))
        # Parameter-driven forces
        vx[n//2,:] += t1.real
        vy[:,n//2] += t1.imag
        vort = np.zeros((n,n))
        
        for k in range(n):
            # Advection-diffusion simulation
            vort = 0.99*vort + 0.01*(np.roll(vx,1,axis=0) - np.roll(vx,-1,axis=0)
                                    - np.roll(vy,1,axis=1) + np.roll(vy,-1,axis=1))
            # Convert vorticity slice to coefficients
            cf = vort[k,:] + 1j*vort[:,k]
            cf /= np.max(np.abs(cf)) + 1e-6
        
        return cf.astype(np.complex128) * np.exp(-np.linspace(0,3,n))
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_astro(t1, t2):
    """Cosmic microwave background-inspired fluctuations"""
    try:
        n = ps.poly.get("n") or 71
        # Spherical harmonics with parameter-driven modes
        theta = np.linspace(0, np.pi, n)
        phi = np.linspace(0, 2*np.pi, n)
        l = int(np.abs(t1) % 10)
        m = int(np.abs(t2) % (l+1))
        
        cf = sph_harm(m, l, phi, theta).real + \
             1j*sph_harm(m, l, phi, theta).imag
        # Add Gaussian random fluctuations
        cf += 0.1*(np.random.randn(n) + 1j*np.random.randn(n))
        return cf.astype(np.complex128) * np.exp(-(theta**2 + phi**2)/4)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_metamaterial(t1, t2):
    """Negative-index photonic crystal band structure"""
    try:
        n = ps.poly.get("n") or 71
        # Create dielectric contrast pattern
        ε = 1 + 0.5*(np.sign(np.sin(10*abs(t1)*np.pi*np.arange(n)/n)) + 
                    np.sign(np.cos(8*abs(t2)*np.pi*np.arange(n)/n)))
        
        # Solve 1D photonic crystal dispersion
        M = np.diag(2*ε) - np.diag(ε[:-1],1) - np.diag(ε[1:],-1)
        eigvals = np.linalg.eigvalsh(M)
        return eigvals + 1j*np.gradient(eigvals)
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_sync(t1, t2):
    """Kuramoto oscillator synchronization dynamics"""
    try:
        n = ps.poly.get("n") or 71
        θ = np.linspace(0, 2*np.pi, n)
        ω = 1 + 0.1*(t1.real*np.cos(θ) + t2.imag*np.sin(θ))
        K = 0.1*abs(t1*t2)
        
        for _ in range(10):  # Temporal evolution
            dθ = ω + K*np.mean(np.sin(θ[:,None] - θ), axis=1)
            θ += 0.1*dθ
        
        return np.exp(1j*θ) * np.linspace(1,0.5,n)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_memristor(t1, t2):
    """Brain-inspired memristive network dynamics"""
    try:
        n = ps.poly.get("n") or 71
        W = np.outer(np.linspace(0,1,n), np.ones(n))  # Memristance matrix
        V = t1.real*np.sin(np.linspace(0,2*np.pi,n)) + \
            t2.imag*np.cos(np.linspace(0,2*np.pi,n))
        
        for _ in range(5):  # Network updates
            I = W @ V
            W += 0.01*(np.outer(V,I) - 0.1*W)  # Hebbian-like update
            V = np.tanh(I) * np.exp(1j*np.angle(I))
        
        return np.diag(W) + 1j*np.diag(np.fft.fft(W))
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_swarm(t1, t2):
    """Particle swarm optimization trajectories"""
    try:
        n = ps.poly.get("n") or 71
        # Initialize particles
        pos = np.random.randn(n) + 1j*np.random.randn(n)
        vel = 0.1*(np.random.randn(n) + 1j*np.random.randn(n))
        pbest = pos.copy()
        gbest = np.mean(pos)
        
        for _ in range(5):  # Optimization steps
            # Update velocities
            vel = 0.5*vel + \
                0.3*(pbest - pos) + \
                0.2*(gbest - pos) * (t1.real + 1j*t2.imag)
            pos += vel
            # Update best positions
            pbest = np.where(np.abs(pos) < np.abs(pbest), pos, pbest)
            gbest = np.mean(pbest[np.argmin(np.abs(pbest))])
        
        return pos * np.hamming(n)
    except:
        return np.zeros(0, dtype=np.complex128)


def poly_cogniverse(t1, t2):
    """Hyperdimensional computing memory superposition"""
    try:
        n = ps.poly.get("n") or 71
        # Create semantic vectors
        v1 = np.random.randn(n) + 1j*np.random.randn(n)
        v2 = np.random.randn(n) + 1j*np.random.randn(n)
        v1 /= np.linalg.norm(v1)
        v2 /= np.linalg.norm(v2)
        
        # Bind and bundle operations
        bound = v1 * np.roll(v2, int(np.abs(t1)%n)) * np.exp(1j*cmath.phase(t2))
        bundled = (bound + np.roll(bound, int(np.abs(t2)%n))) / 2
        
        # Cleanup memory
        for _ in range(3):
            bundled = np.fft.ifft(np.fft.fft(bundled)**2).conj()
        
        return bundled * np.exp(1j*np.linspace(0,8*np.pi,n))
    except:
        return np.zeros(0, dtype=np.complex128)

def poly_sandpile(a, b):
    """Self-organized criticality patterns"""
    n = ps.poly.get("n") or 71
    grid = np.zeros((n,n), dtype=np.complex128)
    grid[n//2, n//2] = a*1000
    
    # Sandpile dynamics
    while True:
        unstable = np.where(grid >= 4)
        if len(unstable[0]) == 0: break
        for i,j in zip(*unstable):
            grid[i,j] -= 4
            if i > 0: grid[i-1,j] += 1
            if i < n-1: grid[i+1,j] += 1
            if j > 0: grid[i,j-1] += 1
            if j < n-1: grid[i,j+1] += 1
    
    return np.poly(grid.diagonal()) * (1 + 1j*b)

def poly_spinglass(a, b):
    """Sherrington-Kirkpatrick spin glass model"""
    n = ps.poly.get("n") or 71
    J = np.random.randn(n,n) * a
    J = (J + J.T)/np.sqrt(n)  # Symmetric couplings
    h = np.random.randn(n) * b  # Random fields
    
    # Parisi replica symmetry breaking
    λ = np.linalg.eigvalsh(J)
    return np.poly(λ + 1j*h[:len(λ)]) * np.exp(-np.arange(n)/10)



def skewed_random(a, size=1):
    u = np.random.uniform(0, 1, size)
    if a == 0:
        return u 
    elif a >= 1:
        return np.zeros_like(u)  
    else:
        return u ** (1 / (1 - a))
    
def random_bunched(a: float) -> float:
    if not (0 <= a <= 1):      
        raise ValueError("Parameter 'a' must be in the interval [0,1].")
    if a == 1:
        return 0.0
    u = np.random.rand()
    exponent = 1.0 / (1.0 - a)
    return u ** exponent

def nopoly_crazy1(t1,t2):
    i = ps.poly.get("i") or 0
    i = i + 1
    ps.poly["i"] = i
    x = (i % 71)/5
    y = (i % 101)/3
    ascii = ps.poly.get("ascii") or 2
    offset = x*np.exp(1j*2*np.pi*y)
    key = f"b{ascii}"
    rts = letters.square(key,0.1*t1,0.1*t2,offset)
    return rts


def nopoly_letter(t1,t2):
    ascii = ps.poly.get("ascii") or 2
    ro = ps.poly.get("ro") or 0
    io = ps.poly.get("io") or 0
    offset = ro + 1j*io
    key = f"b{ascii}"
    rts = letters.square(key,t1,t2,offset)
    return rts

def poly_letter_old(t1,t2):
    ascii = ps.poly.get("ascii") or 2
    ro = ps.poly.get("ro") or 0
    io = ps.poly.get("io") or 0
    norm =  ps.poly.get("norm") or False
    offset = ro + 1j*io
    key = f"b{ascii}"
    rts = letters.square(key,t1,t2,offset)
    mrt = np.max(np.abs(rts))+1
    cf = np.poly(rts/mrt if norm else rts).astype(complex)
    return cf.astype(np.complex128) 

def poly_letter(t1,t2):
    ascii = ps.poly.get("ascii") or 2
    ro = ps.poly.get("ro") or 0
    io = ps.poly.get("io") or 0
    factor = ps.poly.get("factor") or 1.0
    norm =  ps.poly.get("norm") or False
    offset = ro + 1j*io
    key = f"b{ascii}"
    offset = ro +1j*ro
    rts = letters.square(key,t1,t2,0+0j)*factor+offset
    mrt = np.max(np.abs(rts))+1
    cf = np.poly(rts/mrt if norm else rts).astype(complex)
    return cf.astype(np.complex128) 

def poly_letter_2(t1,t2):
    ascii = ps.poly.get("ascii2") or 2
    ro = ps.poly.get("ro2") or 0
    io = ps.poly.get("io2") or 0
    factor = ps.poly.get("factor2") or 1.0
    norm =  ps.poly.get("norm2") or False
    offset = ro + 1j*io
    key = f"b{ascii}"
    offset = ro +1j*ro
    rts = letters.square(key,t1,t2,0+0j)*factor+offset
    mrt = np.max(np.abs(rts))+1
    cf = np.poly(rts/mrt if norm else rts).astype(complex)
    return cf.astype(np.complex128) 

def poly_letter1(t1,t2):
    roots = letters.FONTXY['b2']-1
    pert = 0.1*np.real(t1) * np.exp( 1j*2 * np.pi * np.real(t2) )
    const = 1*np.sin(2*np.pi*t1*np.arange(len(roots)))
    cf = np.poly(roots+pert*(const+np.flip(const))).astype(complex)
    return cf.astype(np.complex128)

def poly_letter_roots(t1,t2):
    cf = letters.square("a",t1,t2,0.+0.j)
    return cf.astype(np.complex128)

def poly_letter_path(t1,t2):
    roots = letters.FONTXY['b2']
    pert = 0.1*np.real(t1) * np.exp( 1j*2 * np.pi * np.real(t2) )
    circles = 1*np.sin(2*np.pi*t1*np.arange(len(roots)))
    symmetrized_circles = (circles+np.flip(circles))
    cf = np.poly( roots + pert * symmetrized_circles ).astype(complex)
    return cf.astype(np.complex128)+1e9*random_bunched(0.999)

def poly_letter2(t1,t2):
    pert = 0.1*np.real(t1) * np.exp( 1j*2 * np.pi * np.real(t2) )
    roots1 = letters.FONTXY['x']+10
    circles1 = 0.1*np.sin(2*np.pi*t1*np.arange(len(roots1)))
    symmetrized_circles1 = (circles1+np.flip(circles1))
    cf1 = np.poly( roots1 + pert * symmetrized_circles1 ).astype(complex)
    roots2 = letters.FONTXY['v']-10
    circles2 = 0.1*np.sin(2*np.pi*t1*np.arange(len(roots2)))
    symmetrized_circles2 = (circles2+np.flip(circles2))
    cf2 = np.poly( roots2 + pert * symmetrized_circles2 ).astype(complex)
    a = bimodal_skewed(0.999)
    cf = cf1 * a + cf2 * (1-a)
    return cf.astype(np.complex128)

def test_letter(t1,t2):
    cf1 = letters.square( 'b2',  t1, t2, 0.+0.j   )
    cf2 = letters.square( 'b3',  t1, t2, 10.+0.j  )
    cf3 = letters.circle( 'b4',  t1, t2, 20.+0.j  )
    cf4 = letters.square( 'b5',  t1, t2, 0.+10.j  )
    cf5 = letters.square( 'b6',  t1, t2, 10.+10.j )
    cf6 = letters.square( 'b7',  t1, t2, 20.+10.j )
    cf7 = letters.square( 'b8',  t1, t2, 0.+20.j  )
    cf8 = letters.square( 'b9',  t1, t2, 10.+20.j )
    cf9 = letters.square( 'b10', t1, t2, 20.+20.j )
    return np.concatenate([
        cf1,
        cf2,
        cf3,
        cf4,
        cf5,
        cf6,
        cf7,
        cf8,
        cf9
    ])

def pad_vector(vec, target_length, pad_value=0+0j):
    return np.pad(vec, (0, target_length - len(vec)), constant_values=pad_value)


def combine(t1,t2):
    poly1 = ps.poly.get("poly1") or "none"
    if poly1=="none": return np.zeros(0, dtype=np.complex128)
    poly2 = ps.poly.get("poly2") or "none"
    if poly2=="none": return np.zeros(0, dtype=np.complex128)
    andy = ps.poly.get("andy") or 0.0
    cf1=globals().get(poly1)(t1,t2)
    cf2=globals().get(poly2)(t1,t2)
    max_length = max(len(cf1), len(cf2))
    pcf1 = pad_vector(cf1, max_length)
    pcf2 = pad_vector(cf2, max_length)
    return pcf1 * andy + pcf2 * ( 1 - andy )

def bimodal_skewed(a, size=1):
    u = np.random.uniform(0, 1, size)
    skewed = np.where(
        u < 0.5, 
        (2*u)**(1/(1-a)) / 2, 
        1 - (2*(1-u))**(1/(1-a))/2
    )
    return np.clip(skewed, 0, 1)

def path(t1,t2):
    bim = ps.poly.get("bim") or 0.99999999
    poly1 = ps.poly.get("poly1") or "none"
    if poly1=="none": return np.zeros(0, dtype=np.complex128)
    poly2 = ps.poly.get("poly2") or "none"
    if poly2=="none": return np.zeros(0, dtype=np.complex128)
    andy = bimodal_skewed(bim)
    cf1=globals().get(poly1)(t1,t2)
    cf2=globals().get(poly2)(t1,t2)
    max_length = max(len(cf1), len(cf2))
    pcf1 = pad_vector(cf1, max_length)
    pcf2 = pad_vector(cf2, max_length)
    return pcf1 * andy + pcf2 * ( 1 - andy )

import solve
import zfrm
import polyflow as pf

sbs_zfrm = None

def sidebyside(t1,t2):
    global sbs_zfrm
    poly1 = ps.poly.get("poly1") or "none"
    if poly1=="none": return np.zeros(0, dtype=np.complex128)
    poly2 = ps.poly.get("poly2") or "none"
    if poly2=="none": return np.zeros(0, dtype=np.complex128)
    sbs_zfrm_str = ps.poly.get("sbs_zfrm") or "none"
    if sbs_zfrm is None:
        sbs_zfrm = ps.get_function_vector(zfrm,sbs_zfrm_str)
    rts1=solve.polyroot(pf.zfrm(globals().get(poly1)(t1,t2),sbs_zfrm))*0.1
    rts2=solve.polyroot(pf.zfrm(globals().get(poly2)(t1,t2),sbs_zfrm))*0.1
    ro = ps.poly.get("ro") or 0
    io = ps.poly.get("io") or 0
    a = ro+1j*io
    rts = np.concatenate((rts1,a+rts2))
    cf = np.poly(rts)
    return cf.astype(np.complex128)

############################################
# side-by-side transform
############################################

def nopoly_lissajous(t1, t2):
    try:
        n = ps.poly.get("n") or 70
        roots = []
        delta = np.angle(t1*t2)
        for k in range(n):
            t = 2*np.pi*k/n
            x = np.abs(t1)*np.sin(t + delta)
            y = np.abs(t2)*np.sin(2*t + delta)
            roots.append(x + 1j*y)
        return np.array(roots,dtype=np.complex128)
    except Exception as e:
        print(f"Exception message: {e}")
        return np.zeros(n, dtype=np.complex128)

def poly_lis2(t1,t2):
    N=71
    curve = []
    delta = np.pi/2
    a = 6
    b = 5
    t0 = 2*np.pi*np.random.uniform(0, 1)
    for k in range(N):
        t = t0+2.0*np.pi*float(k)/float(N)
        x = np.sin(a*t)
        y = np.sin(b*t + delta)
        curve.append(x + 1j*y)
    rpert = np.cumsum(np.random.choice([-1, 1], size=len(curve)+1))
    ipert = np.cumsum(np.random.choice([-1, 1], size=len(curve)+1)) * 1j
    coeffs = np.poly(curve)+0.15*(rpert+ipert)
    return np.array(coeffs,dtype=np.complex128)

def poly_chess(t1,t2):
    N = 7
    w = 2*np.pi
    t = np.random.uniform(-w, w)
    x =  np.sin(t) + np.tile(np.arange(1, N+1), N)
    y =  np.cos(t) + np.repeat(np.arange(1, N+1), N)
    curve = np.array(x + 1j*y,dtype=np.complex128) - ((N+1)/2) - 1j * ((N+1)/2)
    coeffs = np.poly(curve+np.cos(np.random.uniform(-w, w)))
    return np.array(coeffs+90*np.arange(len(coeffs))+np.poly(curve)*200,dtype=np.complex128)

import polychess as pc
import xfrm
import zfrm

def polar_interpolation(x, y, a, geometric_modulus=False):
    r1, theta1 = np.abs(x), np.angle(x)
    r2, theta2 = np.abs(y), np.angle(y)
    dtheta = theta2 - theta1
    dtheta = (dtheta + np.pi) % (2 * np.pi) - np.pi  # Adjust for wrapping
    theta = theta1 + a * dtheta
    if geometric_modulus:
        r = r1**(1 - a) * r2**a
    else:
        r = (1 - a) * r1 + a * r2
    return r * (np.cos(theta) + 1j * np.sin(theta))

def normalize(x):
    return x/np.max(np.abs(x))

def chess2poly(t1,t2):
    andy = ps.poly.get("andy") or 0.0
    n=2
    p = int(andy*n)
    a = (andy*n)%1
    if p==0:
        uct1, uct2 = xfrm.uc(t1,t2)
        cf1 = zfrm.rev(poly_giga_5(uct1,uct2))
    elif p==1:
        a=1-a
        uct1, uct2 = xfrm.uc(t1,t2)
        cf1 = np.poly(np.roots(poly_giga_1(uct1,uct2))*2.5)
    rts1 = np.roots(cf1)
    srts1 = rts1[np.argsort(np.abs(rts1))]
    rts2 = pc.random_chess1(len(rts1),t1,t2)
    srts2 = rts2[np.argsort(np.abs(rts2))]
    rts = polar_interpolation(srts1, srts2, a,geometric_modulus=True)
    # rts = srts1 * (1-andy) + srts2 * andy
    cf = rts
    return np.array(cf,dtype=np.complex128)

def chess2poly1(t1,t2):
    andy = ps.poly.get("andy") or 0.0
    uct1, uct2 = xfrm.uc(t1,t2)
    cf1 = normalize(np.poly(np.roots(poly_giga_1(uct1,uct2))*2.5))
    rts2 = pc.random_chess1(len(cf1)-1,t1,t2)
    cf2 = normalize(np.poly(rts2))
    cf = polar_interpolation(cf1, cf2, andy,geometric_modulus=True)
    return np.array(cf,dtype=np.complex128)

def chess2poly2(t1,t2):
    andy = ps.poly.get("andy") or 0.0
    uct1, uct2 = xfrm.uc(t1,t2)
    cf1 = normalize(np.poly(np.roots(poly_giga_1(uct1,uct2))*2.5))
    rts2 = pc.random_chess1(len(cf1)-1,t1,t2)
    cf2 = normalize(np.poly(rts2))
    a = andy*(t1+t2)/2
    cf = polar_interpolation(cf1, cf2, a ,geometric_modulus=True)
    return np.array(cf,dtype=np.complex128)


def poly_chess1(t1,t2):
    N=7
    def p2(cf):
        n = len(cf)
        cf1 = np.full(n, 1.0, dtype=np.complex128)
        cf2 = ( cf**2 + cf + cf1 ) 
        return cf2.astype(np.complex128)
    w = 2*np.pi
    t = np.random.uniform(-w, w)
    x =  np.sin(t) + np.tile(np.arange(1, N+1), N)
    y =  np.cos(t) + np.repeat(np.arange(1, N+1), N)
    curve = np.array(x + 1j*y,dtype=np.complex128) - ((N+1)/2) - 1j * ((N+1)/2)
    coeffs = np.poly(curve+0.1*np.cos(np.random.uniform(-w, w)))
    cf1 = coeffs
    cf2 = np.pad(curve,(0,1),constant_values=10j)
    cf3 = (cf1+cf2*0.0001)
    cf = cf3 + 0.00000000000000000000001*p2(cf3)
    return np.array(cf,dtype=np.complex128)

def poly_chess2(
    t1,t2,
    N: int = 8
) -> np.ndarray:
    indices = np.arange(N) - (N - 1) / 2
    parity = (np.indices((N, N)).sum(axis=0)) % 2
    X, Y = np.meshgrid(indices, indices)
    t1 = 0.5 * np.exp(1j*2*np.pi*np.random.rand())* parity
    t2 = 0.5 *(np.random.rand()-0.5)* parity
    cf1 = np.poly(( (X + t1) + 1j * (Y + t1)).flatten())   
    cf2 = np.poly(( (X + t2) + 1j * (Y + t2)).flatten())   
    a = np.random.rand()
    coeffs = cf2 * a + cf1 * (1-a)
    return coeffs.astype(complex)

def poly_chess3(
        t1,t2,
        N: int = 8
    ) -> np.ndarray:
    indices = np.arange(N) - (N - 1) / 2
    parity = (np.indices((N, N)).sum(axis=0)) % 2
    X, Y = np.meshgrid(indices, indices)
    t0 = np.random.rand()
    t1 = 0.5 * np.exp(1j*2*np.pi*t0)* parity
    t2 = 0.5 *(t0-0.5)* parity
    cf1 = np.poly(( (X + t1) + 1j * (Y + t1)).flatten())   
    cf2 = np.poly(( (X + t2) + 1j * (Y + t2)).flatten())   
    a = bimodal_skewed(0.85)
    coeffs = cf2 * a + cf1 * (1-a)
    return coeffs.astype(complex)

def spindle(t, a=0.5, b=0.2, p=1.5):
    theta = 2 * np.pi * t
    x = a * np.sign(np.cos(theta)) * np.abs(np.cos(theta))**(2/p)
    y = b * np.sign(np.sin(theta)) * np.abs(np.sin(theta))**(2/p)
    return x + 1j * y

def cardioid(t):
    a = ps.poly.get("crdd") or 0.5
    theta = 2 * np.pi * t
    r = a * (1 + np.cos(theta))
    return r * np.exp(1j * theta)

def heart(u):
    phi = np.pi/2
    t = 2*np.pi*u+phi
    x = 16 * np.sin(t)**3
    y = 13 * np.cos(t) - 5 * np.cos(2*t) - 2 * np.cos(3*t) - np.cos(4*t)
    rot = np.exp(-1j * np.pi / 4)  # e^(-iπ/4)
    hrt = x/40 + 1j*y/40 + 0.1j
    return hrt*rot

def limacon(t, a=0.3, b=0.5):
    theta = 2 * np.pi * t
    r = a + b * np.cos(theta)
    return r * np.exp(1j * theta)

def rose_curve(t, a=0.5, k=2):
    theta = 2 * np.pi * t
    r = a * np.cos(k * theta)
    return r * np.exp(1j * theta)

def lissajous(t, A=0.5, B=0.5, a=3, b=2, delta=np.pi/2):
    theta = 2 * np.pi * t
    x = A * np.sin(a * theta + delta)
    y = B * np.sin(b * theta)
    return x + 1j * y

def astroid(t, a=0.5):
    theta = 2 * np.pi * t
    x = a * np.cos(theta)**3
    y = a * np.sin(theta)**3
    return x + 1j * y

def archimedean_spiral(t, a=0.1, b=0.1):
    theta = 2 * np.pi * t
    r = a + b * theta
    return r * np.exp(1j * theta)

def logarithmic_spiral(t, a=0.1, b=0.1):
    theta = 2 * np.pi * t
    r = a * np.exp(b * theta)
    return r * np.exp(1j * theta)

def deltoid(t, R=1.0):
    theta = 2 * np.pi * t
    x = R * (2 * np.cos(theta) + np.cos(2 * theta)) / 3
    y = R * (2 * np.sin(theta) - np.sin(2 * theta)) / 3
    return x + 1j * y

def ipolygon(t, n=3, radius=1.0, offset=0.0):
    n = ps.poly.get("iplgn") or 3
    t = np.atleast_1d(t)
    t_scaled = t.real * n
    edge_idx = np.floor(t_scaled).astype(int)
    frac = t_scaled - edge_idx
    angles = 2 * np.pi * np.arange(n) / n + offset
    vertices = radius * np.exp(1j * angles)
    v0 = vertices[edge_idx % n]
    v1 = vertices[(edge_idx + 1) % n]
    points = (1 - frac) * v0 + frac * v1
    return points[0] if points.size == 1 else points

def opolygon(t, n=3, radius=1.0, offset=0.0):
    n = ps.poly.get("oplgn") or 3
    t = np.atleast_1d(t)
    t_scaled = t.real * n
    edge_idx = np.floor(t_scaled).astype(int)
    frac = t_scaled - edge_idx
    angles = 2 * np.pi * np.arange(n) / n + offset
    vertices = radius * np.exp(1j * angles)
    v0 = vertices[edge_idx % n]
    v1 = vertices[(edge_idx + 1) % n]
    points = (1 - frac) * v0 + frac * v1
    return points[0] if points.size == 1 else points

def inner(t,factor=0.2):
    inner_line =  t - 0.5 
    inner_circle = factor * np.exp( 1j * 2 * np.pi * t )
    return inner_line + inner_circle

def circle(t):
    crc = np.exp( 1j * 2 * np.pi * t )
    return crc

def split_uniform(t):
    u = t
    v = (t*1000)%1
    return u,v

def disk(t):
    u, v = split_uniform(t)
    r = np.sqrt(u)
    theta = 2 * np.pi * v
    return r * np.exp(1j * theta)

def poly_chess4(
    t1,t2
) -> np.ndarray:
    N = ps.poly.get("n") or 8
    a = ps.poly.get("a") or 0.85
    mod = ps.poly.get("mod") or 2
    off = ps.poly.get("off") or 0
    phi= ps.poly.get("phi") or 0.0
    tt = ps.poly.get("tt") or "tt"
    # inner
    ispeed = ps.poly.get("ispeed")
    if ispeed is None: ispeed = 1.0
    irad = ps.poly.get("irad")
    if irad is None: irad = 0.5
    iname = ps.poly.get("iname") or "circle"
    ifun = globals().get(iname)
    # outer
    ospeed = ps.poly.get("ospeed") 
    if ospeed is None: ospeed = 1.0
    orad = ps.poly.get("orad")
    if orad is None: orad = 0.5
    oname = ps.poly.get("oname") or "circle"
    ofun = globals().get(oname)
    #
    indices = np.arange(N) - (N - 1) / 2
    parity = (((np.indices((N, N)).sum(axis=0))+off) % mod != 0 ).astype(int)
    X, Y = np.meshgrid(indices, indices)
    mask = parity.astype(bool)
    if tt=="t1t1":
        tt1=t1
        tt2=t1
    elif tt=="t1t2":
        tt1 = t1
        tt2 = t2
    elif tt=="t1pmt2":
        tt1 = t1+t2
        tt2 = t1-t2    
    else:
        tt1=np.random.rand()
        tt2=tt1
    to = orad * ofun(tt1 * ospeed )
    ti = irad * ifun(tt2 * ispeed + phi ) 
    cfi = np.poly(( (X[mask] + ti) + 1j * (Y[mask] + ti)).flatten())   
    cfo = np.poly(( (X[mask] + to) + 1j * (Y[mask] + to)).flatten())   
    b = bimodal_skewed(a)
    coeffs = cfo * b + cfi * (1-b)
    return coeffs.astype(complex)

rloc1 = """
 STSTSTSTSTSTSTST
 ST            ST
 ST    SS      STTT
 ST    SS      STTT
 ST    SS      STTT
 ST            ST
 STSTSTSTSTSTSTST
"""

rloc13 = """
_


TTTTTTTTTTTTTTTTTTTTT
T                   S
T      S     T      S
T      S     T      S
T      S     T      S
T      S     T      S
T      S     T      S
T      S     T      S
T      S     T      S    
T                   S
SSSSSSSSSSSSSSSSSSSSS
"""

rjail = """
TTTTTTTTTTT
TSSSSSSSSST
TSSS   SSST      S
TS   T   ST
TSSS   SSST
TSSSSSSSSST
TTTTTTTTTTT
"""

rjail1 = """
SSSSSSSSSSS
STTTTTTTTTS
STTT   TTTS       
ST   S   TS      T
STTT   TTTS
STTTTTTTTTS
SSSSSSSSSSS
"""

rjail2 = """
STSTSTSTSTSTST
ST          ST
ST     S    ST       T
ST          ST
STSTSTSTSTSTST
"""

rjail3 = """
TTTTTTTTTTT
TSSSSSSSSST
TSSS   SSST      
TS   S   ST      T
TSSS   SSST
TSSSSSSSSST
TTTTTTTTTTT
"""

rp1 = """
     TSTS
     TSTS
     TSTS
T    TSTS    S
     TSTS
     TSTS
     TSTS
"""

rp2 = """
     TSTSSS
     TSTS
  TTTTSTS
  TTTTSTS                     SSSSS
  TTTTSTS
     TSTS
     TSTSSS
"""

rp3 = """
     TSTSSS
     STSTS
  TTTTSTS
  TTTTSTS         T     SSSSS     T
  TTTTSTS
     STSTS
     TSTSSS
"""

rp4 = """
TTTTTTTSSSSSSS
"""

rloc6 = """
SSSSSS  TTTTTT
"""

rjail6 = """
ST
ST
ST
SST       T
ST
ST
ST
"""

rjail7 = """
TS
TS
TS
TTS       S
TS
TS
TS
"""

rjail5 = """
S    T
S   T
STS  T
TSSSSSTTT              T
STS  T
S   T
S    T
"""

rjail7 = """
T    S
T   S
TST  S
STTTTTSSS              S
TST  S
T   S
T    S
"""

rjail8 = """
SST         TS     
SST         TS
SST         TST
SSTT  TTT   TST
SST         TST
SST         TS
SST         TS
"""

rloc7 = """


 T T T T T T T T T
S S S S S S S S S T 
      S  T
       S  T
         S
"""

rloc8 = """

TT  TT  TT  TT  TT
S S S S S S S S S S 



         SSSSS




 TT  TT  TT  TT  TT
S S S S S S S S S S 




          TTTTT



  TT  TT  TT  TT  TT
S S S S S S S S S S 

"""

rloc9 = """
TTTTTTTT

TT  TT  TT  TT  TT
S S S S S S S S S S 


SS   SS   SS   SS


 TT  TT  TT  TT  TT
S S S S S S S S S S 
  TT  TT  TT  TT  TT
S S S S S S S S S S 

"""

rloc10 = """

T T T T T T
 S S S S S S
T T T T T T
 S S S S S S
T T T T T T
 S S S S S S

"""

rloc11 = """
         ST
       STSTST         
       TSTSTS
       TSTSTS
       TSTSTS
       TSTSTS
      TSTSTSTS  
     TSTSTSSTST
   STSTSTTSSTSTST
  STTSSTTSTSSTSTST  
 STTSST      STTSST  
"""

rloc_zigzag = """
  TSS
  T S                                                    S
  T S                                                    S
  T S
    T S
      T S S
 S       T S T                           T      S                 T
      T S
    T S                                                  T
  T S                                                    T
 TTS
"""

import polylayout as pl
sX, sY, tX, tY = pl.layout2coord(rloc6)
shape_fun = circle

def poly_chess5_old(t1,t2) -> np.ndarray:
    global sX, sY, tX, tY
    phi = ps.poly.get("phi") or 0.0
    rho = ps.poly.get("rho") or 0.33
    speed = ps.poly.get("speed") or 1.0
    a = ps.poly.get("a") or 0.75
    i = ps.poly.get("i") or 0
    if i==0:
        rloc = ps.poly.get("rloc") or "rloc6"
        x=globals().get(rloc)
        sX, sY, tX, tY = pl.layout2coord(x)
    t = np.random.rand()
    t1 = rho * circle(t)
    t2 = rho * circle(t*speed+phi)
    srts = (sX + t1) + 1j * (sY + t1)   
    trts = (tX + t2) + 1j * (tY + t2)
    scfs = np.poly(srts)
    tcfs = np.poly(trts)
    a = bimodal_skewed(a)
    coeffs = tcfs * a + scfs * (1-a)
    return coeffs.astype(complex)

def poly_chess5(t1,t2) -> np.ndarray:
    global sX, sY, tX, tY, shape_fun
    phi = ps.poly.get("phi") or 0.0
    rho = ps.poly.get("rho") or 0.33
    speed = ps.poly.get("speed") or 1.0
    a = ps.poly.get("a") or 0.75
    i = ps.poly.get("i") or 0
    if i==0:
        shape_name = ps.poly.get("shape_name") or "circle"
        shape_fun = globals().get(shape_name)
        rloc = ps.poly.get("rloc") or "rloc6"
        x=globals().get(rloc)
        sX, sY, tX, tY = pl.layout2coord(x)
    t = np.random.rand()
    t1 = rho * shape_fun(t)
    t2 = rho * shape_fun(t*speed+phi)
    srts = (sX + t1) + 1j * (sY + t1)   
    trts = (tX + t2) + 1j * (tY + t2)
    scfs = np.poly(srts)
    tcfs = np.poly(trts)
    a = bimodal_skewed(a)
    coeffs = tcfs * a + scfs * (1-a)
    return coeffs.astype(complex)


def poly_path(t1,t2) -> np.ndarray:
    global sX, sY, tX, tY, shape_fun
    phi = ps.poly.get("phi") or 0.0
    rho = ps.poly.get("rho") or 0.33
    speed = ps.poly.get("speed") or 1.0
    loc = ps.poly.get("loc") or 0.0
    a = ps.poly.get("a") or 0.75
    i = ps.poly.get("i") or 0
    if i==0:
        shape_name = ps.poly.get("shape") or "circle"
        shape_fun = globals().get(shape_name)
        layout = ps.poly.get("layout") or "rloc6"
        x=globals().get(layout)
        sX, sY, tX, tY = pl.layout2coord(x)
    #t = np.random.rand()
    t = t1.real
    t1 = rho * shape_fun(t)
    t2 = rho * shape_fun(t*speed+phi)
    srts = (sX + t1) + 1j * (sY + t1)   
    trts = (tX + t2) + 1j * (tY + t2)
    prts = srts * (1-loc) + trts * loc
    scfs = np.poly(srts)
    pcfs = np.poly(prts)
    a = bimodal_skewed(a)
    coeffs = pcfs * a + scfs * (1-a)
    return coeffs.astype(complex)


def poly_o3_1(
        t1,t2,
        N: int = 7,
        phase: float = 0.1, 
        amplitude: float = 10, 
        freq: float = 5.0, 
        twist: float = 5.0,
        seed: int = None
    ) -> np.ndarray:
    if seed is not None:
        np.random.seed(seed)
    SMALL_FACTOR = 1e-50
    CURVE_SCALE  = 1e-10
    PADDING_VALUE = 10j
    w = 2 * np.pi
    grid = np.linspace(-1, 1, N)
    X, Y = np.meshgrid(grid, grid)
    r = np.sqrt(X**2 + Y**2)
    theta = np.arctan2(Y, X)
    theta_mod = theta + twist * np.log(r + 1e-6)
    X_mod = r * np.cos(theta_mod)
    Y_mod = r * np.sin(theta_mod)
    x = np.sin(phase + freq * X_mod) + X_mod
    y = np.cos(phase + freq * Y_mod) + Y_mod
    curve = (x + 1j * y).flatten()
    perturbation = 0.1 * np.cos(np.random.uniform(-w, w)) * amplitude
    roots = curve + perturbation
    base_coeffs = np.poly(roots)
    padded_curve = np.pad(curve, (0, 1), constant_values=PADDING_VALUE)
    adjusted_coeffs = base_coeffs + CURVE_SCALE * padded_curve
    def transform_coeffs(cf: np.ndarray) -> np.ndarray:
        return cf.astype(np.complex128)**2 + cf + np.ones_like(cf, dtype=np.complex128) + 0.2 * cf**3
    final_coeffs = adjusted_coeffs + SMALL_FACTOR * transform_coeffs(adjusted_coeffs)
    
    return final_coeffs.astype(complex)

def poly_pacman(t1,t2):
    N = ps.poly.get("n") or 11
    real_coeffs =np.random.choice([-5, 1], size=N+1)  # Random real part
    imag_coeffs = np.random.choice([-1, 5], size=N+1) # Random imaginary part
    coeffs = real_coeffs + 1j * imag_coeffs  # Combine into complex coefficients
    coeffs = np.cumsum(coeffs)*np.exp(1j*2*np.pi*np.random.rand())
    coeffs = coeffs[np.argsort(np.abs(coeffs))]
    coeffs = coeffs * np.flip(np.arange(1,N+2))
    rts = np.roots(coeffs)
    fac = np.arange(1,len(rts)+2)
    addon = 2+5*np.flip(np.exp(1j*2*t1*np.pi*fac))
    #padded_addon  = np.pad(addon,(0,len(rts)),constant_values=1)
    combined = np.concatenate((rts,addon))
    cf = np.poly(combined)
    return cf.astype(np.complex128)



cf_start =  np.zeros(10, dtype=np.complex128)
cf_end =  np.zeros(10, dtype=np.complex128)

def poly_rnd_path1(t1,t2) -> np.ndarray:
    global cf_start, cf_end
    i = ps.poly.get("i") or 0
    if i==0:
        a = ps.poly.get("a") or 1
        b = ps.poly.get("b") or 0
        cf_start = ps.json2cvec(ps.poly["cf_start"])
        cf_end = np.poly(np.roots(ps.json2cvec(ps.poly["cf_end"])) * a+b)
    real_part = (1.0 - t1) * cf_start.real + t1 * cf_end.real
    imag_part = (1.0 - t2) * cf_start.imag + t2 * cf_end.imag
    coeffs = real_part + 1j * imag_part
    return coeffs.astype(complex)


def poly_rnd_path2(t1,t2) -> np.ndarray:
    global cf_start, cf_end
    i = ps.poly.get("i") or 0
    if i==0:
        a = ps.poly.get("a") or 1
        b = ps.poly.get("b") or 0
        cf_start = ps.json2cvec(ps.poly["cf_start"])
        cf_end = np.poly(np.roots(ps.json2cvec(ps.poly["cf_end"])) * a+b)
    t = np.random.uniform(-1, 1)
    real_part = (1.0 - t1 * t ) * cf_start.real + t1 * t * cf_end.real   
    imag_part = (1.0 - t2 * t ) * cf_start.imag + t2 * t * cf_end.imag 
    coeffs = real_part + 1j * imag_part
    return coeffs.astype(complex)

def poly_rnd_path3(t1,t2) -> np.ndarray:
    global cf_start, cf_end
    i = ps.poly.get("i") or 0
    if i==0:
        a = ps.poly.get("a") or 1
        b = ps.poly.get("b") or 0
        cf_start = ps.json2cvec(ps.poly["cf_start"])
        cf_end = np.poly(np.roots(ps.json2cvec(ps.poly["cf_end"])) * a+b)
    ta = np.random.uniform(-1, 1)
    tb = np.random.uniform(-1, 1)
    real_part = (1.0 - t1 * ta ) * cf_start.real + t1 * ta * cf_end.real   
    imag_part = (1.0 - t2 * tb ) * cf_start.imag + t2 * tb * cf_end.imag 
    coeffs = real_part + 1j * imag_part
    return coeffs.astype(complex)

def poly_rnd_path4(t1,t2) -> np.ndarray:
    global cf_start, cf_end
    i = ps.poly.get("i") or 0
    if i==0:
        a = ps.poly.get("a") or 1
        b = ps.poly.get("b") or 0
        cf_start = ps.json2cvec(ps.poly["cf_start"])
        cf_end = np.poly(np.roots(ps.json2cvec(ps.poly["cf_end"])) * a+b)
    ta = np.random.uniform(0, 1)
    tb = np.random.uniform(0, 1)
    real_part = (1.0 - t1 * ta ) * cf_start.real + t1 * ta * cf_end.real   
    imag_part = (1.0 - t2 * tb ) * cf_start.imag + t2 * tb * cf_end.imag 
    coeffs = real_part + 1j * imag_part
    return coeffs.astype(complex)

c = 1
def poly_rnd_path5(t1,t2) -> np.ndarray:
    global cf_start, cf_end, c
    i = ps.poly.get("i") or 0
    if i==0:
        a = ps.poly.get("a") or 1
        b = ps.poly.get("b") or 0
        c = ps.poly.get("c") or 1
        cf_start = ps.json2cvec(ps.poly["cf_start"])
        cf_end = np.poly(np.roots(ps.json2cvec(ps.poly["cf_end"])) * a+b)
    ta = np.random.uniform(-1, 1) * c
    tb = np.random.uniform(-1, 1) * c
    real_part = (1.0 - (t1 * ta) ) * cf_start.real + (t1 * ta) * cf_end.real   
    imag_part = (1.0 - (t2 * tb) ) * cf_start.imag + (t2 * tb) * cf_end.imag 
    coeffs = real_part + 1j * imag_part
    return coeffs.astype(complex)

def poly_rnd_path6(t1,t2) -> np.ndarray:
    global cf_start, cf_end, c
    i = ps.poly.get("i") or 0
    if i==0:
        a = ps.poly.get("a") or 1
        b = ps.poly.get("b") or 0
        c = ps.poly.get("c") or 1
        cf_start = ps.json2cvec(ps.poly["cf_start"])
        cf_end = np.poly(np.roots(ps.json2cvec(ps.poly["cf_end"])) * a+b)
    ta = np.random.uniform(-1, 1)
    tb = np.random.uniform(-1, 1)
    real_part = (1.0 - (t1) ) * cf_start.real + (t1) * cf_end.real
    imag_part = (1.0 - (t2) ) * cf_start.imag + (t2) * cf_end.imag
    coeffs = (real_part + 1j * imag_part)
    return coeffs.astype(complex)

def poly_rnd_path7(t1,t2) -> np.ndarray:
    global cf_start, cf_end, c
    i = ps.poly.get("i") or 0
    if i==0:
        n = ps.poly.get("n") or 10
        a1 = ps.poly.get("a1") or 1
        a2 = ps.poly.get("a2") or 1
        b1 = ps.poly.get("b1") or 0
        b2 = ps.poly.get("b2") or 0
        cf_start = np.poly(ps.random_coeff(n) * a1 + b1)
        cf_end = np.poly(ps.random_coeff(n) * a2 + b2)
    t = np.random.uniform(-1, 1)    
    real_part = (1.0 - (t1) ) * cf_start.real + (t1) * cf_end.real
    imag_part = (1.0 - (t2) ) * cf_start.imag + (t2) * cf_end.imag
    coeffs  = (real_part + 1j * imag_part)
    return coeffs.astype(complex)

def companion_matrix(coeffs):
    coeffs = np.atleast_1d(coeffs).astype(np.complex128)
    n = coeffs.size - 1
    coeffs = coeffs / coeffs[0]
    C = np.zeros((n, n), dtype=np.complex128)
    if n > 1:
        C[:-1, 1:] = np.eye(n - 1)
    C[-1, :] = -coeffs[:0:-1]
    return C.astype(np.complex128)

def poly_giga_140(t1, t2):
    try:
        n = 5
        fac = 1.0/(n+1.0)
        x , y = 2.0 * ( np.indices(((n,n)))/(n-1) - 0.5 )
        z = (x + 1j*y).astype(np.complex128)
        v = (t1 + 1j*t2)
        vv = fac*np.exp(1j*2*np.pi*v)
        cf = np.poly( (z + vv ).flatten())
        cm = companion_matrix(cf).astype(np.complex128)
        a = np.abs(cm)%1
        acf = [1,1j,0,0]
        ap = np.polyval([1,1j,0,0],a)
        tcm = ( 1.0*(cm) - 0.75*( ap ) ).astype(np.complex128)
        roots = np.linalg.eigvals(tcm).astype(np.complex128)
        cf = np.poly( roots ).astype(np.complex128)
        return cf.astype(np.complex128)
    except:
        return np.zeros(5,dtype=np.complex128)
    
def poly_giga_141(t1, t2):
    n = 5
    try:
        fac = 4/(n+1.0)
        x , y = 2.0 * ( np.indices((n,n))/(n-1) - 0.5 )
        z = (x + 1j*y).astype(np.complex128)
        s = fac * t2 * heart(t1) * np.exp(-1j * 2 * np.pi * np.angle(z) )
        cf = np.poly( np.ravel( z + s ) ) 
        cm = companion_matrix(cf).astype(np.complex128)
        a = np.abs(cm)%1
        acf = [0.0*1j,0]
        ap = np.polyval(acf,a)
        cmcf = [1,0]
        cmp = np.polyval(cmcf,cm)
        tcm = ( cmp+ap ).astype(np.complex128)
        roots = np.linalg.eigvals(tcm).astype(np.complex128)
        cf = np.poly( roots ).astype(np.complex128)
        return cf.astype(np.complex128)
    except:
        return np.zeros(n,dtype=np.complex128)
    