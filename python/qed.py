import unittest
import _sequant as sq
from _sequant import Tensor, Sum, Product, Constant, Expr, zRational
from _sequant.mbpt import A,H,T,T_,VacuumAverage

from _sequant.mbpt import Hep, Hw, opH

print(dir(sq))
print(dir(sq.mbpt))

print("\nH_1=", H(1).latex, "\n")
print("\nH_2=", H(2).latex, "\n")
print("\nH_1+H_2=", H().latex, "\n")
print("\nopH_1+H_2=", opH().latex, "\n")

print("\nT(2)=", T(2).latex, "\n")
print("\nT_(2)=", T_(2).latex, "\n")

print("\nBosonic Hamiltonian Hw2=", Hw(2).latex, "\n")
print("\nBosonic Hamiltonian Hep=", Hep().latex, "\n")

print("\nA(-2)=", A(-2).latex, "\n")
print("\nA(2)=", A(2).latex, "\n")

ccsd = VacuumAverage( A(-2) * H() * T(2) * T(2), [(1, 2), (1, 3)] );
print("\nE_ccsd=", ccsd.latex, "\n")



