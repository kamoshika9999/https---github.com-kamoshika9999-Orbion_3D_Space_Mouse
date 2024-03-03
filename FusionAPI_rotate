#Author-KamoShikaCamper
#Description-
"""
転載禁止
商用利用不可
"""

import adsk.core, adsk.fusion, adsk.cam, traceback
import threading
import math
import time
import serial
import numpy

def run(context):
    ui:adsk.core.UserInterface = None
    app:adsk.core.Application = adsk.core.Application.get()
    ui  = app.userInterface
    x_rotMatrix:adsk.core.Matrix3D = adsk.core.Matrix3D.create()
    y_rotMatrix:adsk.core.Matrix3D = adsk.core.Matrix3D.create()

    try:
        ser = serial.Serial("COM3", 9600, timeout=1)
        print(ser.name)
        print("開始")
        flg2 = True
        XValue:int = 0
        YValue:int = 0
        SValue:int = 0
        while flg2:
            flg:bool = True
            while flg:
                S_flg = False
                X_flg = False
                Y_flg = False
                #time.sleep(0.1)
                serStr = ser.read_all()
                result =  serStr.decode().split("\r")
                if len(result) > 1:
                    result = result[0].split(",")
                    for res in result:
                        if "=" in res:
                            if 'S' in res and not(S_flg):
                                print(res)
                                SValue = int(res.split('=')[1])
                                S_flg = True
                                flg = False
                            if 'X' in res and not(X_flg):
                                XValue = int(res.split('=')[1])
                                X_flg = True
                                flg = False
                            if 'Y' in res and not(Y_flg):
                                YValue = int(res.split('=')[1])
                                Y_flg = True
                                flg = False
                    
                adsk.doEvents()

            # アクティブなビューポートを取得
            vp:adsk.core.Viewport = app.activeViewport
            #カメラのコピーを取得
            MyCamera:adsk.core.Camera = vp.camera
            #カメラ更新時スムーズ移動を抑制
            MyCamera.isSmoothTransition = False
            #カメラパラメータ取得
            eye: adsk.core.Point3D = MyCamera.eye
            vec: adsk.core.Vector3D = MyCamera.upVector
            target:adsk.core.Point3D = MyCamera.target
            #ループ用変数
            angle:int = 0
            #回転角度のステップ
            x_angle_step:float =XValue;
            y_angle_step:float =YValue;

            #回転行列設定 テストコードなのでY軸中心に回転
            x_rotMatrix.setToRotation( math.radians(x_angle_step),
                            adsk.core.Vector3D.create(0,1,0),
                            adsk.core.Point3D.create(0,0,0))
            y_rotMatrix.setToRotation( math.radians(y_angle_step),
                            adsk.core.Vector3D.create(1,0,0),
                            adsk.core.Point3D.create(0,0,0))

            eye.transformBy(x_rotMatrix)
            vec.transformBy(x_rotMatrix)
            target.transformBy(x_rotMatrix)
            eye.transformBy(y_rotMatrix)
            vec.transformBy(y_rotMatrix)
            target.transformBy(y_rotMatrix)

            MyCamera.eye = eye
            MyCamera.upVector = vec
            MyCamera.target = target
            #変更した視点のカメラの割り当て
            vp.camera = MyCamera
            #画面強制更新
            vp.refresh()
            #Fusionのメッセージ処理　フリーズ防止
            adsk.doEvents()

    except:
        if ui:
            ui.messageBox('Failed:\n{}'.format(traceback.format_exc()))

