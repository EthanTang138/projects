using Mirror;
using UnityEngine;

public class CameraFollow : NetworkBehaviour
{
    [Header("Camera Settings")]
    public Camera playerCamera;     // 引用此玩家专属摄像机（可以是场景内Camera，也可直接做子物体）

    void Start()
    {
        // 如果不是本地玩家，则禁用摄像机，避免多个玩家摄像机相互干扰
        if (!isLocalPlayer && playerCamera != null)
        {
            playerCamera.enabled = false;
        }
    }

    void LateUpdate()
    {
        // 只有本地玩家才更新摄像机位置
        if (!isLocalPlayer || playerCamera == null) return;

        // 跟随玩家
        Vector3 newPos = transform.position;
        newPos.z = -10f; // 通常让摄像机在 Z 轴上固定一定距离
        playerCamera.transform.position = newPos;
    }
}