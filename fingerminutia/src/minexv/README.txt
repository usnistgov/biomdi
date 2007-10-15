CREATED BY: MDI

The Makefile in this directory builds 3 versions of "minexv":

  minexv  - For use with MINEX04 generated templates
  minexov - For use with MINEX04 generated templates
  minex2v - For use with MINEXII generated templates

Note, the first two are built statically linked with the 'libfmr' code, whereas
'minex2v' is dynamically linked.  (so to run minex2v 'libfmr' must be in the
dynamic loader's search path)


Major differences between what minexv, minexov and minex2v has to enforce:

MINEX04
  MAX RECORD LEN = 4500
  CBEFF PID (BOTH OWNER AND TYPE) MUST be 0
  IMPRESSION TYPE MAY = 0,1,2 or 3
  FINGER QUALITY USES 1,25,50,75,100 SCALE
  MINUTIAE QUALITY VALS MUST = 0

OMINEX
  MAX RECORD LEN = 800                         <CHANGE FROM PREV>
  CBEFF PID (BOTH OWNER AND TYPE) MUST be 0
  FINGER QUALITY USES 20,40,60.80,100 SCALE    <CHANGE FROM PREV>
  IMPRESSION TYPE MAY = 0 or 2                 <CHANGE FROM PREV>
  MINUTIAE QUALITY VALS MUST = 0

MINEX2
  MAX RECORD LEN = 800                         
  CBEFF PID (BOTH OWNER AND TYPE) MUST be > 0  <CHANGE FROM PREV>
  IMPRESSION TYPE MAY = 0 or 2
  FINGER QUALITY USES 20,40,60.80,100 SCALE
  MINUTIAE QUALITY VALS MAY >= 0               <CHANGE FROM PREV>
