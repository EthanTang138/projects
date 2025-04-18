using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using Mirror;

public class Player : NetworkBehaviour
{
    [Header("UI Reference")]
    public HealthBar healthBar;
    public int maxHealth = 100;
    [SyncVar(hook = nameof(OnHealthChanged))]
    public int currentHealth;
    private bool isDead = false;
    public float reviveDelay = 5f;

    void Start()
    {
        if (isServer)
        {
            currentHealth = maxHealth;
        }

        if (isLocalPlayer && healthBar != null)
        {
            healthBar.SetMaxHealth(maxHealth);
            healthBar.SetHealth(currentHealth);
        }
    }

    [Command]
    public void CmdTakeDamage(int damage)
    {
        ServerTakeDamage(damage);
    }

    [Server]
    public void ServerTakeDamage(int damage)
    {
        if (isDead) return;

        // Check if player has an active shield
        PlayerSkills skills = GetComponent<PlayerSkills>();
        if (skills != null && skills.IsShieldActive())
        {
            Debug.Log("[Server] Damage blocked by shield!");
            return; // Cancel damage
        }

        currentHealth -= damage;
        currentHealth = Mathf.Clamp(currentHealth, 0, maxHealth);
        Debug.Log("[Server] Player took damage: " + damage + " | Current Health: " + currentHealth);

        if (currentHealth <= 0)
        {
            Debug.Log("[Server] Player is dead!");
            isDead = true;
            PlayerStats stats = GetComponent<PlayerStats>();
            if (stats != null)
                stats.AddDeath(); // Track death here
            DisablePlayer(); // Stop movement & input
            StartCoroutine(ReviveAfterDelay());
        }
    }
    
    [Command]
    public void CmdHeal(int amount)
    {
        if (!isServer) return;
        if (isDead) return;

        currentHealth += amount;
        currentHealth = Mathf.Clamp(currentHealth, 0, maxHealth);

        Debug.Log($"[Server] Player healed: {amount}, currentHealth={currentHealth}");
    }

    private System.Collections.IEnumerator ReviveAfterDelay()
    {
        yield return new WaitForSeconds(reviveDelay);

        currentHealth = maxHealth;
        isDead = false;
        EnablePlayer();
        Debug.Log("[Server] Player has revived!");
    }

    void OnHealthChanged(int oldValue, int newValue)
    {
        if (healthBar != null)
        {
            healthBar.SetHealth(newValue);
        }
    }

    [Command]
    public void CmdDamageEnemy(GameObject enemy, int damage)
    {
        if (enemy == null) return;  // 已被销毁
        EnemyHealth enemyHealth = enemy.GetComponent<EnemyHealth>();
        if (enemyHealth == null) return; // 脚本或对象不存在
        if (enemyHealth.currentHealth <= 0) return; // 已经死了，不再处理

        enemyHealth.TakeDamage(damage, gameObject);
    }
    private void DisablePlayer()
    {
        // Example: disable your movement script
        GetComponent<PlayerController>().enabled = false;

        // Optionally, play a death animation or make the sprite gray
        // GetComponent<SpriteRenderer>().color = Color.gray;
    }


    private void EnablePlayer()
    {
        // Re-enable movement
        GetComponent<PlayerController>().enabled = true;

        // Restore normal visuals
        // GetComponent<SpriteRenderer>().color = Color.white;
    }
}