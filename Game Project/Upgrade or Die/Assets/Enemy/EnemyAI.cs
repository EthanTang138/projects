using UnityEngine;
using Mirror;

public class EnemyAI : NetworkBehaviour
{
    public float speed = 2f;
    public float chaseRange = 5f;
    private Transform target;
    private Rigidbody2D rb;
    private Animator animator;
    private Vector2 movement;

    void Start()
    {
        rb = GetComponent<Rigidbody2D>();
        animator = GetComponent<Animator>();
    }

    void Update()
    {
        if (!isServer) return; // AI logic runs only on the server

        if (target == null)
        {
            FindClosestTarget();
            return;
        }

        float distanceToTarget = Vector2.Distance(transform.position, target.position);

        if (distanceToTarget < chaseRange)
        {
            movement = (target.position - transform.position).normalized;
        }
        else
        {
            movement = Vector2.zero;
        }
        rb.linearVelocity = movement * speed;
        if (movement.x < -0.01f)
        {
            // 走左
            RpcSetAnimatorState("move_left");
        }
        else if (movement.x > 0.01f)
        {
            // 走右
            RpcSetAnimatorState("move_right");
        }
        else
        {
            // idle
            RpcSetAnimatorState("idle");
        }
    }
    [ClientRpc]
    void RpcSetAnimatorState(string stateName)
    {
        if (!animator) return;

        animator.ResetTrigger("move_left");
        animator.ResetTrigger("move_right");
        animator.ResetTrigger("idle");
        animator.SetTrigger(stateName);

    }

    [ClientRpc] // Ensure all clients see the enemy moving
    void RpcUpdateClientPosition(Vector3 newPosition)
    {
        if (!isServer)
        {
            transform.position = newPosition;
        }
    }

    void FindClosestTarget()
    {
        GameObject[] possibleTargets = GameObject.FindGameObjectsWithTag("Player");
        float closestDistance = Mathf.Infinity;
        Transform closestTarget = null;

        foreach (GameObject potentialTarget in possibleTargets)
        {
            float distance = Vector2.Distance(transform.position, potentialTarget.transform.position);
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestTarget = potentialTarget.transform;
            }
        }

        if (closestTarget != null)
        {
            target = closestTarget;
        }
    }

    /* [Testing Purpose, enemy get damage when collide with player]*/
    private void OnCollisionEnter2D(Collision2D collision)
    {
        if (!isServer) return; // Only server handles damage

        if (collision.gameObject.CompareTag("Player"))
        {
            Player player = collision.gameObject.GetComponent<Player>();
            if (player != null)
            {
                Debug.Log("Enemy collided with player! Damaging player...");
                player.ServerTakeDamage(20); // Deal damage to player
            }


            /*EnemyHealth enemyHealth = GetComponent<EnemyHealth>();
            if (enemyHealth != null)
            {
                Debug.Log("Enemy collided with player! Taking damage.");
                enemyHealth.TakeDamage(20, collision.gameObject); // Deal 20 damage
            }*/
        }
    }
}