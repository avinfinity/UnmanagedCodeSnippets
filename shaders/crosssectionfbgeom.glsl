#version 400

layout(triangles) in;
layout(line_strip, max_vertices = 2) out;
//layout(triangle_strip, max_vertices = 3) out;

uniform mat4 mvpMatrix;
uniform vec3 planeNormal;
uniform vec3 planeCoeff;
uniform int pType;

in vec4[3] vPosition;
in vec3[3] vDirection;
in vec3[3] vColor;
in float[3] thickness;

out vec3 intersectionPoint  , intersectionNormal , intersectionColor;


bool computeIntersectionWithPlane2( vec3 pos1 , vec3 pos2 , vec3 pos3  , vec3 n1 , vec3 n2 , vec3 n3 , 
                                    vec3 col1 , vec3 col2 , vec3 col3, 
                                    vec3 wt , vec3 planeCoeffs , int planeType , out vec3 end1 ,
								    out vec3 end2 , out vec3 iN1 , out vec3 iN2 , out vec3 iCol1 , out vec3 iCol2 )
{
  float diff1 = pos1[ planeType ] - planeCoeffs[ planeType ];
  float diff2 = pos2[ planeType ] - planeCoeffs[ planeType ];
  float diff3 = pos3[ planeType ] - planeCoeffs[ planeType ];
  
  int p = 0 , n = 0;
  
  if( diff1 < 0 )
  n++;
  else
  p++;
  
  if( diff2 < 0 )
  n++;
  else
  p++;
  
  if( diff3 < 0 )
  n++;
  else
  p++;
  
  if( p == 0 || n == 0 )
  return false;
  
  
  float t1 = diff1 / ( pos1[ planeType ] - pos2[ planeType ]);
  float t2 = diff2 / ( pos2[ planeType ] - pos3[ planeType ]);
  float t3 = diff3 / ( pos3[ planeType ] - pos1[ planeType ]);
  
  float interpWt1 , interpWt2;
  
  if( t1 > 0 && t1  < 1 )
  {
     end1 = pos1 * t1 + pos2 * ( 1 - t1 );
	 
	 iN1 = n1 * t1 + n2 * ( 1 - t1 );
	 
	 interpWt1 = wt[ 0 ] * t1 + wt[ 1 ] * ( 1 - t1 );
	 
	 iCol1 = col1 * t1 + col2 * ( 1 - t1 );
	 
	 if( t2 > 0 && t2 < 1 )
	 {
	   end2 = pos2 * t2 + pos3 * ( 1 - t2 );
	   
	   iN2 = n2 * t2 + n3 * ( 1 - t2 );
	   
	   iCol2 = col2 * t2  +  col3 * ( 1 - t2 );
	   
	   interpWt2 = wt[ 1 ] * t2 + wt[ 2 ] * ( 1 - t2 );
	 }
	 else
	 {
	   end2 = pos3 * t3 + pos1 * (  1 - t3 );
	   
	   iN2 = n3 * t3 + n1 * ( 1 - t3 );
	   
	   iCol2 = col3 * t3  +  col1 * ( 1 - t3 );
	   
	   interpWt2 = wt[ 2 ] * t3 + wt[ 0 ] * ( 1 - t3 );
	 }
	 
	 
	 return true;
  }
  else if( t2 > 0 && t2 < 1 )
  {
     end1 = pos2 * t2 + pos3 * ( 1 - t2 );
	 end2 = pos3 * t3 + pos1 * (  1 - t3 );
	 
	 iN1 = n2 * t2 + n3 * ( 1 - t2 );
	 iN2 = n3 * t3 + n1 * (  1 - t3 );
	 
	 iCol1 = col2 * t2  +  col3 * ( 1 - t2 );
	 iCol2 = col3 * t3  +  col1 * ( 1 - t3 );
	 
	 interpWt1 = wt[ 1 ] * t2 + wt[ 2 ] * ( 1 - t2 );
	 interpWt2 = wt[ 2 ] * t3 + wt[ 0 ] * ( 1 - t3 );

	 return true;
  }
  else
  {
    return false;
  }
 
}

void main()
{
    vec3 intersectEnd1 , intersectEnd2 , iN1 , iN2 , iCol1 , iCol2;
	
	vec3 wt;
	
	wt[ 0 ] = thickness[ 0 ];
	wt[ 1 ] = thickness[ 1 ];
	wt[ 2 ] = thickness[ 2 ];
	
	
	/*&& computeIntersectionWithPlane2( vPosition[0].xyz , vPosition[1].xyz , vPosition[2].xyz ,
                                                                                    vDirection[ 0 ] , vDirection[ 1 ] , vDirection[ 2 ]  , vColor[0], 
																				    vColor[1], vColor[2] , wt , planeCoeff , pType , intersectEnd1 , 
																				    intersectEnd2 , iN1 , iN2 , iCol1 , iCol2 ) )*/
	
	
	if( wt[ 0 ] > 0 && wt[ 1 ] > 0 && wt[ 2 ] > 0 )
	{
       vec4 v0 = mvpMatrix * vec4( intersectEnd1 , 1 );
       vec4 v1 = mvpMatrix * vec4( intersectEnd2 , 1 );

	   gl_Position = v0;
	   
	   intersectionPoint = intersectEnd1;
	   
	   iN1[ pType ] = 0;
	   
	   normalize( iN1 );
	   
	   intersectionNormal = iN1;
	   
	   intersectionColor = iCol1;
       
       EmitVertex();
	   
       gl_Position = v1;
	   
	   intersectionPoint = intersectEnd2;
	   
	   iN2[ pType ] = 0;
	   
	   normalize( iN2 );
	   
	   intersectionNormal = iN2;
	   
	   intersectionColor = iCol2;
	   
       EmitVertex();
	   
	   EndPrimitive();
	}

}

