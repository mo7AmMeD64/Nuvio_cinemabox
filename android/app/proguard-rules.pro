# O SDL e o nucleo C chamam estas classes por JNI: nada de renomear/remover.
-keep class org.libsdl.app.** { *; }
-keep class space.nuvio.nativelegacy.** { *; }
