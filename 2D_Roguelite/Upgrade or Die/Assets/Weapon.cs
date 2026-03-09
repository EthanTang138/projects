using UnityEngine;

public class Weapon : MonoBehaviour
{
public int damage = 10;

private GameObject wielder;

     void Start()
    {
                wielder = transform.root.gameObject; 
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (other.CompareTag("Enemy"))
        {
            other.GetComponent<EnemyHealth>().TakeDamage(damage,wielder);
        }
}
}
