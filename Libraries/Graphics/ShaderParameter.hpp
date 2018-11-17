///////////////////////////////////////////////////////////////////////////////
///
/// \file SceneEffect.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
#define SetParam(name) {ShaderParameter p = shader->GetParameter(#name); shader->SetParameter(p, name); }

#define ApplyParam(name)  apply(shader, #name, name); 

template<typename type>
class ShaderParam
{
public:
  operator type&(){return value;}
  ShaderParameter Param;
  void operator=(const type& newValue){value = newValue;}
  type value;
};

struct ApplyToShaderFunc
{
  template<typename type>
  void operator()(Shader* shader, cstr name, ShaderParam<type>& param)
  {
    shader->SetParameter(param.Param, param.value);
  }
};

struct LoadShaderParamter
{
  template<typename type>
  void operator()(Shader* shader, cstr name, ShaderParam<type>& param)
  {
    param.Param = shader->GetParameter(name);
  }
};

template<typename type>
void Apply(type& data, Shader* shader)
{
  data.Visit(shader, ApplyToShaderFunc());
}

template<typename type>
void Load(type& data, Shader* shader)
{
  data.Visit(shader, LoadShaderParamter());
}

}