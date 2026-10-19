import math,sys
def rtri(v,r):
    # rounded polygon path (convex, CCW or CW) with fillet radius r (or list)
    n=len(v); out=[]
    rs=r if isinstance(r,list) else [r]*n
    pts=[]
    for i in range(n):
        p0=v[i-1];p=v[i];p1=v[(i+1)%n];rr=rs[i]
        a=(p0[0]-p[0],p0[1]-p[1]);b=(p1[0]-p[0],p1[1]-p[1])
        la=math.hypot(*a);lb=math.hypot(*b)
        a=(a[0]/la,a[1]/la);b=(b[0]/lb,b[1]/lb)
        ang=math.acos(max(-1,min(1,a[0]*b[0]+a[1]*b[1])))
        t=rr/math.tan(ang/2)
        pts.append(((p[0]+a[0]*t,p[1]+a[1]*t),(p[0]+b[0]*t,p[1]+b[1]*t),rr))
    d=""
    for i,(s,e,rr) in enumerate(pts):
        d+=("M" if i==0 else "L")+"%.2f %.2f "%s
        # sweep: determine by cross
        p0=v[i-1];p=v[i];p1=v[(i+1)%n]
        cr=(p[0]-p0[0])*(p1[1]-p[1])-(p[1]-p0[1])*(p1[0]-p[0])
        d+="A%.2f %.2f 0 0 %d %.2f %.2f "%(rr,rr,1 if cr>0 else 0,e[0],e[1])
    return d+"Z"
OUT=[(662,128),(1111,408),(662,688)]
HOLE=[(757,304),(932,408),(759,514)]
WH=[(795,362),(872,408),(795,454)]
outer=rtri(OUT,76); hole=rtri(HOLE,24); wh=rtri(WH,9)
facets='''
<polygon points="600,100 1200,100 1020,326 928,404 762,312 650,214" fill="url(#gt)"/>
<polygon points="928,404 1020,326 1200,326 1200,760 777,760 777,512" fill="url(#gr)"/>
<polygon points="650,214 762,312 932,406 777,512 650,563 560,563 560,214" fill="url(#gl)"/>
<polygon points="650,563 777,512 777,760 560,760 560,563" fill="url(#gb)"/>'''
defs='''<defs>
<linearGradient id="gt" gradientUnits="userSpaceOnUse" x1="760" y1="130" x2="1000" y2="380"><stop offset="0" stop-color="#12F2FF"/><stop offset="1" stop-color="#1B9BFF"/></linearGradient>
<linearGradient id="gr" gradientUnits="userSpaceOnUse" x1="880" y1="330" x2="1010" y2="560"><stop offset="0" stop-color="#8A3BFF"/><stop offset="1" stop-color="#E24DFF"/></linearGradient>
<linearGradient id="gl" gradientUnits="userSpaceOnUse" x1="662" y1="240" x2="740" y2="540"><stop offset="0" stop-color="#1E7BFF"/><stop offset="1" stop-color="#5B33F2"/></linearGradient>
<linearGradient id="gb" gradientUnits="userSpaceOnUse" x1="680" y1="520" x2="760" y2="640"><stop offset="0" stop-color="#4B2BE0"/><stop offset="1" stop-color="#7A3BFF"/></linearGradient>
<clipPath id="cp"><path d="%s"/></clipPath></defs>'''%outer
# symbol bbox
X0,Y0,W,H=650,170,420,480  # tight-ish around rounded shape (662..1043, 196..616)
def sym(vb):
    return '<g clip-path="url(#cp)">'+facets+'</g>'
# hole: paint bg? Need transparent hole -> use evenodd path as clip instead
mk='<mask id="mk" maskUnits="userSpaceOnUse" x="500" y="0" width="800" height="800"><path d="%s" fill="#fff"/><path d="%s" fill="#000"/></mask>'%(outer,hole)
defs=defs.replace('</defs>',mk+'</defs>')
body='<g mask="url(#mk)">'+facets+'</g><path d="%s" fill="#fff"/>'%wh
open('symbol.body','w').write(defs+body)
bx=(662,196,1043,616)
print(bx)
