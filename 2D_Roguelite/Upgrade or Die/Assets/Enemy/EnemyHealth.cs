using Mirror;
using UnityEngine;
using UnityEngine.UI;

public class EnemyHealth : NetworkBehaviour
{
    public GameObject healthBarPrefab;
    private Slider healthBarSlider;

    [SyncVar(hook = nameof(OnHealthChanged))]
    public int currentHealth = 100;

    public int maxHealth = 100;

    public GameObject lastAttacker;

    private Transform canvasTransform;

    public override void OnStartServer()
    {
        currentHealth = maxHealth;
    }

    public override void OnStartClient()
    {
        SpawnHealthBar();
    }

    void SpawnHealthBar()
    {
        GameObject bar = Instantiate(healthBarPrefab, transform.position + Vector3.up * 1.2f, Quaternion.identity);
        bar.transform.SetParent(transform);
        healthBarSlider = bar.GetComponentInChildren<Slider>();

        // Normalize the bar range
        healthBarSlider.minValue = 0f;
        healthBarSlider.maxValue = 1f;

        // Update value based on percent
        healthBarSlider.value = 1-((float)currentHealth / maxHealth);
    }

    void OnHealthChanged(int oldHealth, int newHealth)
    {
        // Just in case maxHealth is zero (prevent divide-by-zero)
        float percent = maxHealth > 0 ? (float)newHealth / maxHealth : 0f;

        if (healthBarSlider != null)
            healthBarSlider.value = 1-percent;
    }

    [Server]
    public void TakeDamage(int damage, GameObject attacker)
    {
        currentHealth -= damage;
        currentHealth = Mathf.Clamp(currentHealth, 0, maxHealth);

        lastAttacker = attacker;

        if (currentHealth <= 0)
        {
            if (lastAttacker != null)
            {
                Experience xp = lastAttacker.GetComponent<Experience>();
                if (xp != null) xp.CmdAddXP(50);

                PlayerStats stats = lastAttacker.GetComponent<PlayerStats>();
                if (stats != null) stats.AddKill();
            }

            NetworkServer.Destroy(gameObject);
        }
    }
}
