#include "ProfileWizard.hpp"
#include <ui_ProfileEditPage.h>
#include <ui_ProfileIntroPage.h>
#include <ui_ProfileNymPage.h>

#include <fc/thread/thread.hpp>

#include <fc/log/logger.hpp>

class NymPage : public QWizardPage
{
    public:
        NymPage( QWidget* parent )
        :QWizardPage(parent),_complete(false)
        {
          setTitle( tr( "Create your Keyhotee ID" ) );
          _profile_nym_ui.setupUi(this);

          connect( _profile_nym_ui.keyhotee_id, &QLineEdit::textEdited,
                  this, &NymPage::validateId );
        }

        virtual bool isComplete()const
        {
            return _complete;
        }


        /** starts a lookup timer that will send a query one second
         * after the most recent edit unless another character is
         * entered.
         */
        void validateId( const QString& id )
        {
            _complete = false;
            completeChanged();
            _last_validate = fc::time_point::now();
            _profile_nym_ui.id_warning->setText( tr( "Checking availability of ID..." ) );
            fc::async( [=](){ 
                fc::usleep( fc::microseconds(500*1000) );
                if( fc::time_point::now() > (_last_validate + fc::microseconds(500*1000)) )
                {
                   lookupId();
                }
            } );
        }
        
        void lookupId()
        {
            try {
                auto cur_id = _profile_nym_ui.keyhotee_id->text().toStdString();
                auto opt_name_rec = bts::application::instance()->lookup_name( cur_id );
                if( opt_name_rec )
                {
                     _profile_nym_ui.id_warning->setText( tr( "This ID has been taken by another user" ) );
                }
                else
                {
                     _profile_nym_ui.id_warning->setText( tr( "This ID is available!" ) );
                     _complete = true;
                     completeChanged();
                }
            } 
            catch ( const fc::exception& e )
            {
               _profile_nym_ui.id_warning->setText( e.to_string().c_str() );
            }
        }


        fc::time_point _last_validate;
        bool           _complete;
        Ui::NymPage    _profile_nym_ui;
};

class ProfileEditPage : public QWizardPage
{
   public:
      ProfileEditPage( QWidget* parent )
      :QWizardPage(parent)
      {
          //Init Validation Variables
          noncharacters_present_list << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0" << "~" << "`" << "!" << "@" << "#" << "$" << "%" << "^" << "&" << "*" << "(" << ")" << "-" << "_" << "+" << "=" << "{" << "[" << "]" << "}" << "\\" << "|" << ";" << ":" << "'" << "\"" << ","  << "<" << ">" << "." << "/" << "?";


          //Setup UI
          setTitle( tr( "Create your Keyhotee Profile" ) );
          ui.setupUi(this);

          //Connections
          connect( ui.generaterandom, &QPushButton::clicked,
                   this, &ProfileEditPage::generateSeed );
          
          connect( ui.brainkey, &QLineEdit::textChanged,
                   this, &ProfileEditPage::brainKeyEdited );
          
          connect( ui.local_password1, &QLineEdit::textEdited,
                   this, &ProfileEditPage::loginPasswordEdited );
          
          connect( ui.local_password2, &QLineEdit::textEdited,
                   this, &ProfileEditPage::loginPasswordCheckEdited );
      }

      void generateSeed()
      {
         auto key = fc::ecc::private_key::generate();
         ui.brainkey->setText( std::string(key.get_secret()).c_str() );
         completeChanged();
      }

      void brainKeyEdited( const QString& key )
      {
         if( key.size() < 32 )
         {
            ui.brainkey_warning->setText( tr( "Your Brain Key must be at least 32 characters" ) ); 
         }
         else
         {
            ui.brainkey_warning->setText( QString() ); 
         }
         completeChanged();
      }

      void loginPasswordEdited( const QString& password )
      {
         //Track states
         int local_password2_enabled = 1; //Default 1; Turns off when a warning/error in the inputted password occurs
         QString password_warning_text = QString();


         //Clear password input field 2
         ui.local_password2->setText( QString() );

         //Password checking logic
            int password_size = password.size();

             /** Password Length Check **/
             if( password_size < 8 )
             {
                 password_warning_text.append( tr( "Password must be at least 8 characters" ) );

                 //Flag password two input disabled.
                 local_password2_enabled = 0;
             }

             /** Entropy Check for Letters and numbers used) **/
             //Loop through the entire string until satisfaction.
             int characters_present = 0;
             int noncharacters_present = 0;
             for( int a = 0; a < password_size; a++ )
             {
                //Character or non-character?
                 if(noncharacters_present_list.contains(password.at(a))){
                    noncharacters_present = 1;
                 }else{
                     characters_present = 1;
                 }
             }

             if(characters_present == 0 || noncharacters_present == 0){
                 if(password_warning_text.size() > 0){
                    password_warning_text.append( tr( " and must contain numbers (or symbols)" ) );
                 }else{
                    password_warning_text.append( tr( "Use characters and numbers (or symbols)" ) );
                 }
                 //Flag password two input disabled
                 local_password2_enabled = 0;
             }

             /** Password1 & 2 Exact Match Check **/
             if(local_password2_enabled == 1 && ui.local_password2->text() != ui.local_password1->text())
             {
                 if(password_warning_text.size() > 0){
                     password_warning_text.append( tr( " and passwords do not match"));
                 }else{
                     password_warning_text.append( tr( "Passwords do not match" ) );
                 }
             }

         //Disabled/Enable password 2 input field
         if( local_password2_enabled == 1){
             ui.local_password2->setEnabled(true);
         }else if( local_password2_enabled == 0 ){
             ui.local_password2->setEnabled(false);
         }

         ui.password_warning->setText( password_warning_text );

         completeChanged();
      }



      void loginPasswordCheckEdited( const QString& password )
      {

         if( ui.local_password1->text() == password )
         {
            // TODO: check that password isn't on the commonly used
            ui.password_warning->setText( QString() );
         }
         else
         {
            ui.password_warning->setText( tr( "Passwords do not match" ) );
         }
         completeChanged();
      }
      virtual bool isComplete()const
      {
          if( ui.brainkey->text().size() < 32 ) return false;
          if( ui.local_password1->text().size() < 8 ) return false;
          if( ui.local_password1->text() != ui.local_password2->text() ) return false;
          return true;
      }

      Ui::ProfileEditPage ui;

    private:
        QStringList noncharacters_present_list;
};



ProfileWizard::ProfileWizard( QWidget* parent ) :QWizard(parent)
{          
   setAttribute( Qt::WA_DeleteOnClose );
   setOption(HaveHelpButton, true);

   QWizardPage* intro_page = new QWizardPage(this);
   intro_page->setTitle( tr( "Welcome to Keyhotee" ) );
   _profile_intro_ui = new Ui::IntroPage();
   _profile_intro_ui->setupUi(intro_page);

   _nym_page        = new NymPage(this);
   _profile_edit    = new ProfileEditPage(this);


   connect( this, &ProfileWizard::helpRequested, this, &ProfileWizard::showHelp );

   connect( this, &ProfileWizard::finished, this, &ProfileWizard::createProfile );


   setPage( Page_Intro, intro_page );
   setPage( Page_Profile, _profile_edit );
   setPage( Page_FirstNym, _nym_page );

   setStartId( Page_Intro );

   #ifndef Q_WS_MAC
      setWizardStyle( ModernStyle );
   #else
      setWizardStyle( MacStyle );
   #endif
}

ProfileWizard::~ProfileWizard()
{
   if( !_profile_edit->isComplete() )
   {
        qApp->quit();
   }
}

void ProfileWizard::showHelp()
{
   // TODO: open up the help browser and direct it to this page.
}


void ProfileWizard::createProfile( int result )
{
   if( _profile_edit->isComplete() )
   {
      
   }
}


