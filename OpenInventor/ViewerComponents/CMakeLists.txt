#project(ViewerComponents)

set(source_files
  SceneExaminer.h
  SceneExaminer.cxx
  SceneInteractor.h
  SceneInteractor.cxx
  Animator.h
  Animator.cxx
  AnimatorListener.h
  SeekAnimator.h
  SeekAnimator.cxx
)

add_library( oiviewercomponents ${source_files})

target_link_libraries( oiviewercomponents ${IMT_OPENINVENTOR_LIBRARIES} )
