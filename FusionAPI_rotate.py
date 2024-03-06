#Author-KamoShikaCamper
#Description-
"""
Ver.0.2
2024/03/06 22:14
転載禁止
商用利用不可
"""

import adsk.core, adsk.fusion, adsk.cam, traceback
import math
import serial
import serial.tools.list_ports

def run(context):
    app:adsk.core.Application = adsk.core.Application.get()
    ui:adsk.core.UserInterface = None
 
    ui  = app.userInterface
    x_rotMatrix:adsk.core.Matrix3D = adsk.core.Matrix3D.create()
    y_rotMatrix:adsk.core.Matrix3D = adsk.core.Matrix3D.create()
    shaft_rotMatrix:adsk.core.Matrix3D = adsk.core.Matrix3D.create()

    try:
        ports = list(serial.tools.list_ports.comports())
        for port in ports:
            description=port.description
            if 'Arduino' in description:
                print("Arduino Leonardo = ",port.device)
                ser = serial.Serial(port.device, 115200, timeout=1)
                break
        print("開始")
        flg2 = True
        XValue:int = 0
        YValue:int = 0
        SValue:int = 0
        while flg2:
            des:adsk.fusion.Design = app.activeProduct
            actComp = des.activeComponent
            actTarget:adsk.core.Point3D = adsk.core.Point3D.create(
                (actComp.boundingBox.maxPoint.x + actComp.boundingBox.minPoint.x)/2,
                (actComp.boundingBox.maxPoint.y + actComp.boundingBox.minPoint.y)/2,
                (actComp.boundingBox.maxPoint.z + actComp.boundingBox.minPoint.z)/2)
            flg:bool = True
            while flg:
                S_flg = False
                X_flg = False
                Y_flg = False
                SValue = 0
                serStr = ser.read_all()
                result =  serStr.decode().split("\r")
                if len(result) > 1:
                    result = result[0].split(",")
                    for res in result:
                        if "=" in res:
                            if 'S' in res and not(S_flg):
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
                        elif "JB" in res:
                            ser.close()
                            flg2 = False
                            flg = False
                            print("終了")
                            ui.messageBox("シリアル通信切断")
                            break
                    
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
            shaft_vec: adsk.core.Vector3D = MyCamera.upVector
            #拡大縮小
            extent = MyCamera.getExtents()
            vWidth = extent[1]
            vHight = extent[2]
            #print("vWidth=",vWidth,"vHight=",vHight)
            magnification = 1.2
            if SValue == 1:
                vWidth *= magnification
                vHight *= magnification
            elif SValue == 255:
                vWidth /= magnification
                vHight /= magnification
            MyCamera.setExtents(vWidth,vHight)
            #print("vWidth=",vWidth,"vHight=",vHight)
            #ループ用変数
            angle:int = 0
            #回転角度のステップ
            x_angle_step:float =XValue/5;
            y_angle_step:float =YValue/5;

            #回転行列設定 テストコードなのでY軸中心に回転
            x_rotMatrix.setToRotation( math.radians(x_angle_step),
                            vec,
                            actTarget)#adsk.core.Point3D.create(0,0,0))
            
            
            shaft_rotMatrix.setToRotation(math.radians(90),
                            eye.vectorTo(MyCamera.target),
                            adsk.core.Point3D.create(0,0,0))

            
            shaft_vec.transformBy(shaft_rotMatrix)
            y_rotMatrix.setToRotation( math.radians(y_angle_step),
                            shaft_vec,                            
                            actTarget)#adsk.core.Point3D.create(0,0,0))

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

