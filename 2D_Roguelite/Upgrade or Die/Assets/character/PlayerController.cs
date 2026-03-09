using Mirror;
using UnityEngine;
using System.Collections; 
using UnityEngine.UI; 

public class PlayerController : NetworkBehaviour
{
    public float speed = 5f;  
    public float dodgeDistance = 3f;
    public float dodgeCooldown = 1f; 
    public float invincibleTime = 0.5f;

    private Rigidbody2D rb;
    private Vector2 movement;
    private Animator animator;
    private bool isDodging = false;
    private float lastDodgeTime = -999f;
    private Vector2 dodgeDirection;
    private float o_speed = 0f;
    public GameObject playerUIPrefab; 
    private GameObject playerUIInstance;
    private GameObject inventoryPanel;
    public GameObject interactIcon;
    private bool isOpen = false; 
    private Vector2 boxSize = new Vector2(0.1f, 1f);
    


    public GameObject attackTrianglePrefab;
    public GameObject aimPrefab;
    private GameObject aimObject;
    private GameObject attackTriangleInstance;
    private bool isAttacking = false;
    public Camera localCam; 
    Vector3 mouseWorldPos;
    Vector2 attackDirection;
    void Start()
    {
        rb = GetComponent<Rigidbody2D>();
        animator = GetComponent<Animator>();

        aimObject = Instantiate(aimPrefab, transform);
        aimObject.transform.localPosition = Vector3.zero;

        attackTriangleInstance = Instantiate(attackTrianglePrefab, aimObject.transform);
        attackTriangleInstance.SetActive(false);
        

        // Prevent multiple UI instances
        if (GameObject.Find("PlayerUI(Clone)") != null)
        {
            Debug.LogWarning("Duplicate PlayerUI detected! Skip creating extra instance.");
            return;
        }
        playerUIInstance = Instantiate(playerUIPrefab);
        playerUIInstance.transform.SetParent(GameObject.Find("Canvas").transform, false);

        inventoryPanel = playerUIInstance.transform.Find("InventoryPanel").gameObject;
        inventoryPanel.SetActive(false);

    }

    void Update()
    {
        if (!isLocalPlayer) return; // only allowed to control your own character
        // mouseWorldPos = localCam.ScreenToWorldPoint(Input.mousePosition);
        if(Input.GetKeyDown(KeyCode.F)) // interact button
        {
            CheckInteraction();
        }
        /*
        if (Input.GetKeyDown(KeyCode.Space) && !isDodging && Time.time > lastDodgeTime + dodgeCooldown)
        {
            StartCoroutine(Dodge(movement));
        }*/
        if (!isDodging)
        {
            rb.linearVelocity = movement * speed;
        } 

        if (!isAttacking){
            movement.x = Input.GetAxisRaw("Horizontal"); // A (-1), D (+1)
            movement.y = Input.GetAxisRaw("Vertical");   // W (+1), S (-1)
        }
        animator.ResetTrigger("walking_left");
        animator.ResetTrigger("walking_up");
        animator.ResetTrigger("walking_down");
        animator.ResetTrigger("walking_right");
        animator.ResetTrigger("idle");
        animator.ResetTrigger("attack_up");
        animator.ResetTrigger("attack_down");
        animator.ResetTrigger("attack_left");
        animator.ResetTrigger("attack_right");

        if (movement.x < 0) 
        {
            animator.SetTrigger("walking_left"); 
        }
        else if(movement.y > 0)
        {
            animator.SetTrigger("walking_up");
        }
        else if (movement.x > 0) 
        {
            animator.SetTrigger("walking_right");
        }
        else if ( movement.y < 0)
        {
            animator.SetTrigger("walking_down");
        }
        else
        {
            animator.SetTrigger("idle");
        }

        if (Input.GetKeyDown(KeyCode.I))
        {
            isOpen = !isOpen;
            inventoryPanel.SetActive(isOpen);
        }
        if ((Input.GetMouseButtonDown(0) || Input.GetKeyDown("j")) && !isAttacking)
        {
            movement.x = 0;
            movement.y = 0;
            PerformAttack();
        }
    }
    void PerformAttack()
    {
        if (attackTriangleInstance == null)
        {
            Debug.LogError("attackTriangleInstance is NULL! Make sure it's instantiated in Start().");
            return;
        }

        if (aimObject == null)
        {
            Debug.LogError("aimObject is NULL! Make sure it's instantiated in Start().");
            return;
        }

        isAttacking = true;
        attackTriangleInstance.SetActive(true);
        attackDirection = (mouseWorldPos - transform.position).normalized;
        string attackAnimation = GetAttackDirection(attackDirection);

        switch (attackAnimation) // for damage calc
        {
            case "attack_up":
                aimObject.transform.rotation = Quaternion.Euler(0, 0, 0f);
                break;
            case "attack_down":
                aimObject.transform.rotation = Quaternion.Euler(0, 0, 180f);
                break;
            case "attack_left":
                aimObject.transform.rotation = Quaternion.Euler(0, 0, 90f);
                break;  
            case "attack_right":
                aimObject.transform.rotation = Quaternion.Euler(0, 0, 270f);
                break;
        }
        Debug.Log(attackAnimation);
        animator.SetTrigger(attackAnimation);
        Collider2D[] hits = Physics2D.OverlapCircleAll(transform.position, 1.0f);
        foreach (Collider2D hit in hits)
        {
            if (hit.CompareTag("Enemy"))
            {
                Player playerScript = GetComponent<Player>();
                if (playerScript != null)
                {
                    playerScript.CmdDamageEnemy(hit.gameObject, 20);
                }
            }
        }
        Invoke(nameof(EndAttack), 0.5f); 
    }

string GetAttackDirection(Vector2 dir)
{
    if (Mathf.Abs(dir.x) > Mathf.Abs(dir.y))
    {
        return dir.x > 0 ? "attack_right" : "attack_left";
    }
    else
    {
        return dir.y > 0 ? "attack_up" : "attack_down";
    }
}


    void EndAttack()
    {
        attackTriangleInstance.SetActive(false);
        animator.SetTrigger("idle");
        isAttacking = false;
    }
    
    IEnumerator Dodge(Vector2 direction)
    {
        if (direction == Vector2.zero) 
        {
            direction = Vector2.right; 
        }
        isDodging = true;
        o_speed = speed;
        speed = 0;
        lastDodgeTime = Time.time;
        // **dash**
        Vector2 newPosition = (Vector2)transform.position + direction * dodgeDistance;
        rb.MovePosition(newPosition);

        // **invincible**
        int invincibleLayer = LayerMask.NameToLayer("Invincible");
        if (invincibleLayer != -1)
        {
            gameObject.layer = invincibleLayer;
        }

        yield return new WaitForSeconds(invincibleTime);

        gameObject.layer = LayerMask.NameToLayer("Player");
        isDodging = false;
        speed = o_speed;
    }
        public void OpenInteractableIcon()
    {
        interactIcon.SetActive(true);
    }

    public void CloseInteractableIcon()
    {
        interactIcon.SetActive(false);
    }

    private void CheckInteraction()
    {
        RaycastHit2D[] hits = Physics2D.BoxCastAll(transform.position, boxSize, 0, Vector2.zero);
        if(hits.Length > 0)
        {
            foreach(RaycastHit2D rc in hits)
            {
                if(rc.transform.GetComponent<Interactable>())
                {
                    rc.transform.GetComponent<Interactable>().Interact(gameObject);
                    return;
                }
            }
        }
    }
}