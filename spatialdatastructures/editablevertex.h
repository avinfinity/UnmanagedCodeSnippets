/*
 * Copyright 2014 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef EDITABLEVERTEX_H
#define EDITABLEVERTEX_H


template <class T> class HbrVertexEdit;
template <class T> class HbrMovingVertexEdit;

class EditableVertex
{

protected:

     int mVid;
  
public:
  
    EditableVertex() {}

    EditableVertex(int index) { mVid = index; }

    EditableVertex( EditableVertex const & src ) {}

    void AddWithWeight(EditableVertex const & i, float weight, void * = 0) {  }

    void AddVaryingWithWeight(const EditableVertex & i, float weight, void * = 0) {}

    void Clear(void * = 0) {}

    void ApplyVertexEdit(HbrVertexEdit<EditableVertex> const &) { }

    void ApplyMovingVertexEdit( HbrMovingVertexEdit<EditableVertex> const &) { }
    
    int vId(){ return mVid; }
    
    ~EditableVertex(){};
};

#endif // EDITABLEVERTEX_H
