#version 330

layout(triangles) in;
layout(line_strip, max_vertices = 2) out;

uniform mat4 mvpMatrix;
uniform vec3 planeNormal;
uniform vec3 planeCoeff;
uniform int pType;

in vec4[3] vPosition;
in vec3[3] vNormal;

bool computeIntersectionWithPlane( vec3 pos1 , vec3 pos2 , vec3 pos3  , vec3 n1 , vec3 n2 , vec3 n3 , 
                                   vec3 planeCoeffs , in vec3 wt , int planeType , out vec3 end1 ,
								   out vec3 end2 , out vec3 interpN1 , out vec3 interpN2  )
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
  
  if( t1 > 0 && t1  < 1 )
  {
     end1 = pos1 * t1 + pos2 * ( 1 - t1 );
	 
	 interpN1 = n1 * t1 + n2 * ( 1 - t1 );
	 
	 interpWt1 = wt[ 0 ] * t1 + wt[ 1 ] * ( 1 - t1 );
	 
	 if( t2 > 0 && t2 < 1 )
	 {
	   end2 = pos2 * t2 + pos3 * ( 1 - t2 );
	   
	   interpN2 = n2 * t2 + n3 * ( 1 - t2 );
	   
	   interpWt2 = wt[ 1 ] * t2 + wt[ 2 ] * ( 1 - t2 );
	 }
	 else
	 {
	   end2 = pos3 * t3 + pos1 * (  1 - t3 );
	   
	   interpN2 = n3 * t3 + n1 * ( 1 - t3 );
	   
	 }
	 
	 return true;
  }
  else if( t2 > 0 && t2 < 1 )
  {
     end1 = pos2 * t2 + pos3 * ( 1 - t2 );
	 end2 = pos3 * t3 + pos1 * (  1 - t3 );
	 
	 interpN1 = n2 * t2 + n3 * ( 1 - t2 );
	 interpN2 = n3 * t3 + n1 * ( 1 - t3 );
	 
	 return true;
  }
  else
  {
    return false;
  }
 
}

void createLine( vec4 pos1 , vec4 pos2 )
{
	gl_Position = mvpMatrix * pos1;
       
    EmitVertex();
	   
    gl_Position = mvpMatrix * pos2;
	   
    EmitVertex();
	   
	EndPrimitive();

}

void main()
{


    vec3 intersectEnd1 , intersectEnd2 , end3 , end4;
	
	if( wt[ 0 ] > 0 && wt[ 1 ] > 0 && wt[ 2 ] > 0 && computeIntersectionWithPlane( vPosition[0].xyz , vPosition[1].xyz , vPosition[2].xyz ,
                                                                                    vNormal[ 0 ] , vNormal[ 1 ] , vNormal[ 2 ] , planeCoeff ,wt , 
									                                                pType , intersectEnd1 , intersectEnd2 , end3 , end4 ) )
	{

       vec4 v0 = mvpMatrix * vec4( intersectEnd1 , 1 );
       vec4 v1 = mvpMatrix * vec4( intersectEnd2 , 1 );
	   vec4 v2 = mvpMatrix * vec4( end3 , 1 );
       vec4 v3 = mvpMatrix * vec4( end4 , 1 );
	   
	   gl_Position = v0;
       
       EmitVertex();
	   
       gl_Position = v1;

       EmitVertex();
	   
	   gl_Position = v2;

	   EmitVertex();

	   gl_Position = v2;
	   
	   EmitVertex();
	   
	   gl_Position = v1;
	   
	   EmitVertex();
		
	   gl_Position = v3;
	   
	   EmitVertex();
	   
	   EndPrimitive();
    }

}

