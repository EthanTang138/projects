using UnityEngine;
using Mirror;
using System.Collections;
using TMPro;

public class PlayerSkills : NetworkBehaviour
{
    public Experience experience;  // 请在 Inspector 中将 Experience 脚本拖入

    [Header("Fireball Settings")]
    public GameObject fireballPrefab;
    public float fireballSpeed = 10f;
    public float shieldDuration = 3f;
    public TMP_Text shieldStatusText;
    public float fireballCooldown = 1f;
    public Transform firePoint; 
    private float lastFireTime = -999f;
    [SyncVar]
    private bool isShielded = false;


    void Start()
    {
        if (!isLocalPlayer)
        {
            enabled = false;
            return;
        }
        // 确保 experience 已经赋值
        if (experience == null)
        {
            experience = GetComponent<Experience>();
        }

        if (shieldStatusText != null)
        {
            shieldStatusText.gameObject.SetActive(false);
        }
    }

    void Update()
    {
        if (!isLocalPlayer) return;

        // 使用 Q 键触发 Fireball
        if (Input.GetKeyDown(KeyCode.Q) && experience.unlockedFireball && Time.time > lastFireTime + fireballCooldown)
        {
            Vector3 mousePos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            Vector2 direction = (mousePos - transform.position).normalized;
            CmdCastFireball(direction);
            lastFireTime = Time.time;
        }
        // 使用 E 键触发 Dash
        if (Input.GetKeyDown(KeyCode.Space) && experience.unlockedDash)
        {
            UseDash();
        }
        // 使用 R 键触发 Shield
        if (Input.GetKeyDown(KeyCode.R) && experience.unlockedShield && !isShielded)
        {
            CmdActivateShield();
        }
    }

    [Command]
    void CmdCastFireball(Vector2 dir)
    {
        Vector3 spawnPos = firePoint.position;
        spawnPos.z = -1f; 
        GameObject fireball = Instantiate(fireballPrefab, spawnPos, Quaternion.identity);
        fireball.GetComponent<FireballProjectile>().Initialize(dir, gameObject);
        NetworkServer.Spawn(fireball);
    }

    void UseDash()
    {
        Vector2 dashDirection = new Vector2(Input.GetAxisRaw("Horizontal"), Input.GetAxisRaw("Vertical")).normalized;
        if (dashDirection == Vector2.zero)
            dashDirection = Vector2.right; // 默认向右
        transform.position += (Vector3)dashDirection * 3f;
        Debug.Log("Dash used!");
    }

    [Command]
    void CmdActivateShield()
    {
        StartCoroutine(ShieldRoutine());
    }
    IEnumerator ShieldRoutine()
    {
        isShielded = true;
        TargetShowShieldMessage(connectionToClient, true); // show UI message on local player
        yield return new WaitForSeconds(shieldDuration);
        isShielded = false;
        TargetShowShieldMessage(connectionToClient, false); // hide message
    }

    [TargetRpc]
    void TargetShowShieldMessage(NetworkConnection target, bool isActive)
    {
        if (shieldStatusText != null)
        {
            shieldStatusText.text = isActive ? "Shield Active!" : "";
            shieldStatusText.gameObject.SetActive(isActive);
        }
    }

    public bool IsShieldActive()
    {
        return isShielded;
    }
}
