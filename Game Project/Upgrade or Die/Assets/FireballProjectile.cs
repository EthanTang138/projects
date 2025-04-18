using UnityEngine;
using Mirror;

public class FireballProjectile : NetworkBehaviour
{
    public float speed = 10f;
    public int damage = 25;
    public float lifetime = 3f;

    private Vector2 direction;
    private GameObject owner; 

    public void Initialize(Vector2 dir, GameObject attacker)
    {
        direction = dir.normalized;
        owner = attacker;
        Invoke(nameof(DestroySelf), lifetime);
    }

    void Update()
    {
        if (isServer)
        {
            transform.Translate(direction * speed * Time.deltaTime);
        }
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (!isServer) return;

        if (other.CompareTag("Enemy"))
        {
            EnemyHealth enemy = other.GetComponent<EnemyHealth>();
            if (enemy != null)
            {
                enemy.TakeDamage(damage, owner);// or attacker if needed
            }
            DestroySelf();
        }
    }

    void DestroySelf()
    {
        NetworkServer.Destroy(gameObject);
    }
}
