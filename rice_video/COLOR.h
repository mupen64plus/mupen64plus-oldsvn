#ifndef XCOLOR_H
#define XCOLOR_H

typedef struct _COLORVALUE 
{
   
       float r;
       float g;
       float b;
       float a;
} COLORVALUE;


typedef struct XCOLOR {
   FLOAT r, g, b, a;
#ifdef __cplusplus
 public:
   XCOLOR() 
     {
     }
   
   XCOLOR( DWORD argb );
   XCOLOR( CONST FLOAT * );
   XCOLOR( CONST COLORVALUE& );
   XCOLOR( FLOAT r, FLOAT g, FLOAT b, FLOAT a );
   
   // casting
   operator DWORD () const;
   
   operator FLOAT* ();
   operator CONST FLOAT* () const;
   
   operator COLORVALUE* ();
   operator CONST COLORVALUE* () const;
    
   operator COLORVALUE& ();
   operator CONST COLORVALUE& () const;
   
   // assignment operators
   XCOLOR& operator += ( CONST XCOLOR& );
   XCOLOR& operator -= ( CONST XCOLOR& );
   XCOLOR& operator *= ( FLOAT );
   XCOLOR& operator /= ( FLOAT );
   
   // unary operators
   XCOLOR operator + () const;
   XCOLOR operator - () const;
   
   // binary operators
   XCOLOR operator + ( CONST XCOLOR& ) const;
   XCOLOR operator - ( CONST XCOLOR& ) const;
   XCOLOR operator * ( FLOAT ) const;
   XCOLOR operator / ( FLOAT ) const;
   
   friend XCOLOR operator * (FLOAT, CONST XCOLOR& );
    
   BOOL operator == ( CONST XCOLOR& ) const;
   BOOL operator != ( CONST XCOLOR& ) const;
   
#endif //__cplusplus
} XCOLOR;

#endif
