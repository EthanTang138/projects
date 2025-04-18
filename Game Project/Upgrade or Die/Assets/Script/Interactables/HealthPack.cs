using UnityEngine;
[RequireComponent(typeof(SpriteRenderer))]

public class HealthPack : Interactable
{
    public Sprite consumable;

    private SpriteRenderer sr;
    private bool isConsumed;
    private GameObject player;
    public override void Interact(GameObject interactor)
    {
        Player player = interactor.GetComponent<Player>();
        if(!isConsumed)
        {
            player.CmdHeal(50);
            Destroy(gameObject);
        }
    }

    private void Start()
    {
        sr = GetComponent<SpriteRenderer>();
        isConsumed = false;
    }

}
