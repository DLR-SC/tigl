// Created on: 2013-10-10
// Created by: Denis BOGOLEPOV
// Copyright (c) 2013-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

varying vec3 View;          //!< Direction to the viewer
varying vec3 Normal;        //!< Vertex normal in view space
varying vec4 Position;      //!< Vertex position in view space.
varying vec4 PositionWorld; //!< Vertex position in world space
varying vec4 uv;            //!< Vertex uv coordinate

uniform bool enableZebra; //!< Whether the zebra stripe mode is enabled

vec3 Ambient;  //!< Ambient  contribution of light sources
vec3 Diffuse;  //!< Diffuse  contribution of light sources
vec3 Specular; //!< Specular contribution of light sources

//! Computes contribution of isotropic point light source
void pointLight (in int  theId,
                 in vec3 theNormal,
                 in vec3 theView,
                 in vec3 thePoint)
{
  vec3 aLight = occLight_Position (theId).xyz;
  if (occLight_IsHeadlight (theId) == 0)
  {
    aLight = vec3 (occWorldViewMatrix * occModelWorldMatrix * vec4 (aLight, 1.0));
  }
  aLight -= thePoint;

  float aDist = length (aLight);
  aLight = aLight * (1.0 / aDist);

  float anAtten = 1.0 / (occLight_ConstAttenuation  (theId)
                       + occLight_LinearAttenuation (theId) * aDist);

  vec3 aHalf = normalize (aLight + theView);

  vec3  aFaceSideNormal = gl_FrontFacing ? theNormal : -theNormal;
  float aNdotL = max (0.0, dot (aFaceSideNormal, aLight));
  float aNdotH = max (0.0, dot (aFaceSideNormal, aHalf ));

  float aSpecl = 0.0;
  if (aNdotL > 0.0)
  {
    aSpecl = pow (aNdotH, gl_FrontFacing ? occFrontMaterial_Shininess() : occBackMaterial_Shininess());
  }

  Diffuse  += occLight_Diffuse  (theId).rgb * aNdotL * anAtten;
  Specular += occLight_Specular (theId).rgb * aSpecl * anAtten;
}

//! Computes contribution of spotlight source
void spotLight (in int  theId,
                in vec3 theNormal,
                in vec3 theView,
                in vec3 thePoint)
{
  vec3 aLight   = occLight_Position      (theId).xyz;
  vec3 aSpotDir = occLight_SpotDirection (theId).xyz;
  if (occLight_IsHeadlight (theId) == 0)
  {
    aLight   = vec3 (occWorldViewMatrix * occModelWorldMatrix * vec4 (aLight,   1.0));
    aSpotDir = vec3 (occWorldViewMatrix * occModelWorldMatrix * vec4 (aSpotDir, 0.0));
  }
  aLight -= thePoint;

  float aDist = length (aLight);
  aLight = aLight * (1.0 / aDist);

  aSpotDir = normalize (aSpotDir);

  // light cone
  float aCosA = dot (aSpotDir, -aLight);
  if (aCosA >= 1.0 || aCosA < cos (occLight_SpotCutOff (theId)))
  {
    return;
  }

  float anExponent = occLight_SpotExponent (theId);
  float anAtten    = 1.0 / (occLight_ConstAttenuation  (theId)
                          + occLight_LinearAttenuation (theId) * aDist);
  if (anExponent > 0.0)
  {
    anAtten *= pow (aCosA, anExponent * 128.0);
  }

  vec3 aHalf = normalize (aLight + theView);

  vec3  aFaceSideNormal = gl_FrontFacing ? theNormal : -theNormal;
  float aNdotL = max (0.0, dot (aFaceSideNormal, aLight));
  float aNdotH = max (0.0, dot (aFaceSideNormal, aHalf ));

  float aSpecl = 0.0;
  if (aNdotL > 0.0)
  {
    aSpecl = pow (aNdotH, gl_FrontFacing ? occFrontMaterial_Shininess() : occBackMaterial_Shininess());
  }

  Diffuse  += occLight_Diffuse  (theId).rgb * aNdotL * anAtten;
  Specular += occLight_Specular (theId).rgb * aSpecl * anAtten;
}

//! Computes contribution of directional light source
void directionalLight (in int  theId,
                       in vec3 theNormal,
                       in vec3 theView)
{
  vec3 aLight = normalize (occLight_Position (theId).xyz);
  if (occLight_IsHeadlight (theId) == 0)
  {
    aLight = vec3 (occWorldViewMatrix * occModelWorldMatrix * vec4 (aLight, 0.0));
  }

  vec3 aHalf = normalize (aLight + theView);

  vec3  aFaceSideNormal = gl_FrontFacing ? theNormal : -theNormal;
  float aNdotL = max (0.0, dot (aFaceSideNormal, aLight));
  float aNdotH = max (0.0, dot (aFaceSideNormal, aHalf ));

  float aSpecl = 0.0;
  if (aNdotL > 0.0)
  {
    aSpecl = pow (aNdotH, gl_FrontFacing ? occFrontMaterial_Shininess() : occBackMaterial_Shininess());
  }

  Diffuse  += occLight_Diffuse  (theId).rgb * aNdotL;
  Specular += occLight_Specular (theId).rgb * aSpecl;
}

//! Computes illumination from light sources
vec4 computeLighting (in vec3 theNormal,
                      in vec3 theView,
                      in vec4 thePoint)
{
  // Clear the light intensity accumulators
  Ambient  = occLightAmbient.rgb;
  Diffuse  = vec3 (0.0);
  Specular = vec3 (0.0);
  vec3 aPoint = thePoint.xyz / thePoint.w;
  for (int anIndex = 0; anIndex < occLightSourcesCount; ++anIndex)
  {
    int aType = occLight_Type (anIndex);
    if (aType == OccLightType_Direct)
    {
      directionalLight (anIndex, theNormal, theView);
    }
    else if (aType == OccLightType_Point)
    {
      pointLight (anIndex, theNormal, theView, aPoint);
    }
    else if (aType == OccLightType_Spot)
    {
      spotLight (anIndex, theNormal, theView, aPoint);
    }
    else
    {
      // on some hardware, aType is always zero. 
      // as a workaround, we use the directional light
      directionalLight (anIndex, theNormal, theView);
    }
  }

  vec4 aMaterialAmbient  = gl_FrontFacing ? occFrontMaterial_Ambient()  : occBackMaterial_Ambient();
  vec4 aMaterialDiffuse  = gl_FrontFacing ? occFrontMaterial_Diffuse()  : occBackMaterial_Diffuse();
  vec4 aMaterialSpecular = gl_FrontFacing ? occFrontMaterial_Specular() : occBackMaterial_Specular();
  vec4 aMaterialEmission = gl_FrontFacing ? occFrontMaterial_Emission() : occBackMaterial_Emission();
  float mMaterialTransparency = gl_FrontFacing ? occFrontMaterial_Transparency() : occBackMaterial_Transparency();

  float attenuation = 1.0;
  if (enableZebra)
  {
    vec3 v = vec3(0., 0., -1.);

    // Direction of the view reflected on the surface
    vec3 vReflect = 2. * (dot(theNormal, v)*theNormal - v);

    // normal vector of the light stripe plane
    vec3 lightDir = normalize(vec3(0., 1., 0.));

    // View projected into light plane
    vec3 vProj = normalize(v - dot(v, lightDir)*lightDir);

    // x-position of the reflected view on the light plane
    float posLightPlane = dot(vReflect, vProj);

    attenuation = max(min(2.0, sin(posLightPlane*30.0)*3. + 2.0), -1.0);
  }

  if (occTextureEnable > 0)
  {
    aMaterialDiffuse = aMaterialDiffuse * texture2D(occActiveSampler, uv.xy);
  }

  vec4 color = vec4 (Ambient,  1.0) * aMaterialAmbient
             + vec4 (Diffuse,  1.0) * aMaterialDiffuse
             + vec4 (Specular, 1.0) * aMaterialSpecular
                                  + aMaterialEmission;

  return vec4(color.xyz, mMaterialTransparency) * vec4(attenuation, attenuation, attenuation, 1.0);
}

//! Entry point to the Fragment Shader
void main()
{
  // process clipping planes
  for (int anIndex = 0; anIndex < occClipPlaneCount; ++anIndex)
  {
    vec4 aClipEquation = occClipPlaneEquations[anIndex];
    int  aClipSpace    = occClipPlaneSpaces[anIndex];
    if (aClipSpace == OccEquationCoords_World)
    {
      if (dot (aClipEquation.xyz, PositionWorld.xyz) + aClipEquation.w < 0.0)
      {
        discard;
      }
    }
    else if (aClipSpace == OccEquationCoords_View)
    {
      if (dot (aClipEquation.xyz, Position.xyz) + aClipEquation.w < 0.0)
      {
        discard;
      }
    }
  }

  gl_FragColor = computeLighting (normalize (Normal),
                                  normalize (View),
                                  Position);
}
