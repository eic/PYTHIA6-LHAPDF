C*********************************************************************

C...PYGALF
C...Longitudinal resolved-photon factors VINT(315), VINT(316) for
C...subprocess ISUBX at the current photon kinematics, with Q2 the
C...hard-process scale (used by MSTP(17)=1,2 only).
C...Split out of PYSIGH so that PYRAND can set them before PYMULT(2)
C...and PYMIGN(2), which otherwise read the values left by the
C...previous PYSIGH call (another subprocess and phase-space point).

      SUBROUTINE PYGALF(ISUBX,Q2)

C...Double precision and integer declarations
      IMPLICIT DOUBLE PRECISION(A-H, O-Z)
      IMPLICIT INTEGER(I-N)
      INTEGER PYK,PYCHGE,PYCOMP
C...Commonblocks
      COMMON/PYDAT2/KCHG(500,4),PMAS(500,4),PARF(2000),VCKM(4,4)
      COMMON/PYPARS/MSTP(200),PARP(200),MSTI(200),PARI(200)
      COMMON/PYINT1/MINT(400),VINT(400)
      SAVE /PYDAT2/,/PYPARS/,/PYINT1/
      include "mc_set.inc"
 
C...Extra factors to include the effects of
C...longitudinal resolved photons (but not direct or DIS ones).
      DO 170 ISDE=1,2
        IF(MINT(10+ISDE).EQ.22.AND.MINT(106+ISDE).GE.1.AND.
     &  MINT(106+ISDE).LE.3) THEN
          VINT(314+ISDE)=1D0
          XY=PARP(166+ISDE)
          IF(MSTP(16).EQ.0) THEN
            IF(VINT(304+ISDE).GT.0D0.AND.VINT(304+ISDE).LT.1D0)
     &      XY=VINT(304+ISDE)
          ELSE
            IF(VINT(308+ISDE).GT.0D0.AND.VINT(308+ISDE).LT.1D0)
     &      XY=VINT(308+ISDE)
          ENDIF
          Q2GA=VINT(306+ISDE)
          IF(MSTP(17).GT.0.AND.XY.GT.0D0.AND.XY.LT.1D0.AND.
     &    Q2GA.GT.0D0) THEN
            REDUCE=0D0
            IF(MSTP(17).EQ.1) THEN
              REDUCE=4D0*Q2*Q2GA/(Q2+Q2GA)**2
            ELSEIF(MSTP(17).EQ.2) THEN
              REDUCE=4D0*Q2GA/(Q2+Q2GA)
            ELSEIF(MSTP(17).EQ.3) THEN
              PMVIRT=PMAS(PYCOMP(113),1)
              REDUCE=4D0*Q2GA/(PMVIRT**2+Q2GA)
            ELSEIF(MSTP(17).EQ.4.AND.MINT(106+ISDE).EQ.1) THEN
              PMVIRT=PMAS(PYCOMP(113),1)
              REDUCE=4D0*PMVIRT**2*Q2GA/(PMVIRT**2+Q2GA)**2
            ELSEIF(MSTP(17).EQ.4.AND.MINT(106+ISDE).EQ.2) THEN
              PMVIRT=PMAS(PYCOMP(113),1)
              REDUCE=4D0*PMVIRT**2*Q2GA/(PMVIRT**2+Q2GA)**2
            ELSEIF(MSTP(17).EQ.4.AND.MINT(106+ISDE).EQ.3) THEN
              PMVSMN=4D0*PARP(15)**2
              PMVSMX=4D0*VINT(154)**2
              REDTRA=1D0/(PMVSMN+Q2GA)-1D0/(PMVSMX+Q2GA)
              REDLON=(3D0*PMVSMN+Q2GA)/(PMVSMN+Q2GA)**3-
     &        (3D0*PMVSMX+Q2GA)/(PMVSMX+Q2GA)**3
              REDUCE=4D0*(Q2GA/6D0)*REDLON/REDTRA
            ELSEIF(MSTP(17).EQ.5.AND.MINT(106+ISDE).EQ.1) THEN
              PMVIRT=PMAS(PYCOMP(113),1)
              REDUCE=4D0*Q2GA/(PMVIRT**2+Q2GA)
            ELSEIF(MSTP(17).EQ.5.AND.MINT(106+ISDE).EQ.2) THEN
              PMVIRT=PMAS(PYCOMP(113),1)
              REDUCE=4D0*Q2GA/(PMVIRT**2+Q2GA)
            ELSEIF(MSTP(17).EQ.5.AND.MINT(106+ISDE).EQ.3) THEN
              PMVSMN=4D0*PARP(15)**2
              PMVSMX=4D0*VINT(154)**2
              REDTRA=1D0/(PMVSMN+Q2GA)-1D0/(PMVSMX+Q2GA)
              REDLON=1D0/(PMVSMN+Q2GA)**2-1D0/(PMVSMX+Q2GA)**2
              REDUCE=4D0*(Q2GA/2D0)*REDLON/REDTRA
C ........Hermes version of R_VMD, this still needs work to
C only apply it for process 91 and separate rho from the rest.
            ELSEIF(MSTP(17).EQ.6.AND.MINT(106+ISDE).EQ.2) THEN
             IF ((ISUBX.le.94).and.(ISUBX.gt.90)) then
              if (VINT(67).gt.0.0) then
                if (VINT(67).eq.PMAS(PYCOMP(113),1)) then
                   REDUCE=(Q2GA/VINT(67)**2)**PARP(166)
                else
                   REDUCE=Q2GA/VINT(67)**2
                endif
              else
                PMVIRT=PMAS(PYCOMP(113),1)
                REDUCE=Q2GA/PMVIRT**2
              endif
             ENDIF
            ENDIF
            BEAMAS=PYMASS(11)
            IF(VINT(302+ISDE).GT.0D0) BEAMAS=VINT(302+ISDE)
            IF((MINT(11).EQ.22).and.
     &         (MINT(12).EQ.2212.or.MINT(12).EQ.2112)) THEN
              FRACLT=1D0/(1D0+(XY**2*(1D0-2D0*BEAMAS**2/Q2GA))/
     &         (2D0/(1D0+Q2GA/XY**2/ebeamEnucl**2)*(1D0-XY-
     &         (Q2GA/4D0/ebeamEnucl**2))))
            ELSE
              FRACLT=1D0/(1D0+XY**2/2D0/(1D0-XY)*
     &        (1D0-2D0*BEAMAS**2/Q2GA))
            ENDIF
            VINT(314+ISDE)=1D0+PARP(165)*REDUCE*FRACLT
          ENDIF
        ELSE
          VINT(314+ISDE)=1D0
        ENDIF
  170 CONTINUE
 
      RETURN
      END
